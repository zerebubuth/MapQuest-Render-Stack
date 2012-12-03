#!/usr/bin/env python
#------------------------------------------------------------------------------
#
#  Mapnik Renderer
#
#  Author: john.novak@mapquest.com
#  Author: matt.amos@mapquest.com
#  Author: kevin.kreiser@mapquest.com
#
#  Copyright 2010-1 Mapquest, Inc.  All Rights reserved.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#------------------------------------------------------------------------------

from shapely.wkt import loads
from shapely.prepared import prep
from shapely.geometry import Polygon

from mapnik2 import Map
from mapnik2 import load_map
from mapnik2 import Image
from mapnik2 import render
from mapnik2 import Projection
from mapnik2 import Box2d
from mapnik2 import Coord
from mapnik2 import CompositeOp
from mapnik2 import CreateDatasource

import re

#from tile_pb2 import tile
from mercator import Mercator

#convert to independant format
import PIL.Image
import PIL.ImageOps

#splice out the meta data
from metacutter import extractFeaturesMNK

#returned results in this structure
from renderResult import RenderResult

#for new map stack unified logging
import mq_logging

class Mask :
    def __init__(self, wkt_mask_file):
        wkt = open(wkt_mask_file).read()
        mask = loads(wkt)
        self.mask = prep(mask)
        
    def relate(self, x0, y0, x1, y1):
        p = Polygon([(x0, y0), (x1, y0), (x1, y1), (x0, y1), (x0, y0)])
        return self.mask.intersects(p),self.mask.contains(p)

METATILE = 8
TILE_SIZE = 256
 
class Renderer :
    
    def __init__(self, map_style, mask_style):
        self.default_map = Map(0,0)

        # only load the mask style if it's provided
        if mask_style is None:
            self.mask_map = None
        else:
            self.mask_map = Map(0,0)

        self.proj = Mercator(18+1)
        self.regions = []
        try :
            load_map(self.default_map, map_style)
            
            # masks may not be provided if it's a single style
            if self.mask_map is not None:
                load_map(self.mask_map, mask_style)

            self.map_proj = Projection(self.default_map.srs)
        except:
            mq_logging.error("Exception caught in Renderer ctor")
            raise
        
    def add_region(self,name,mapfile,wkt_mask_file):
        # ignore attempts to add regions when no mask has been
        # specified with a warning.
        if self.mask_map is not None:
            try:
                m = Map(0,0)
                load_map(m, mapfile)
                mask = Mask(wkt_mask_file)
                self.regions.append((name,m,mask))
            except Exception as ex:
                mq_logging.error("Exception caught adding region (%s)" % str(ex))
        else:
            mq_logging.warning("Cannot add mask (%s) because no mask_style was configured." % name)
    
    def _check_region(self, bbox):
        for r in self.regions:
            intersects,contains = r[2].relate(bbox.minx,bbox.miny,bbox.maxx,bbox.maxy)
            if intersects :
                return contains,r[1],r[0]    
        return None

    def save_rendered_metadata(self, renderer, size, dimensions):
        #save off the meta data if its there
	if renderer is not None:
	    inmem = renderer.find_inmem_metawriter("poi-meta-data")
            if inmem is not None:
       	        #get the bounding boxes and ids out of the mapnik data and save in a feature collection
                return extractFeaturesMNK(inmem)    

	#no data so no feature collection
	return None

    # If the tile has a language parameter, change the datasource definition
    # in the map so that the language setting is taken into account. This is
    # currently a bit of a hack, but can be improved later.
    def adjust_language(self, map, languages):
        if languages:
            for layer in self.default_map.layers:
                params = layer.datasource.params().as_dict()
                if params.get('labelhint', '') != '':
                    lc = []
                    for lines in languages.split("|"):
                        langs = []
                        for lang in lines.split(","):
                            if lang == '_':
                                langs.append("name")
                            else:
                                langs.append("tags->'name:" + lang + "'")
                        langs.append("''")
                        lc.append('coalesce(' + ','.join(langs) + ')')
                    if len(lc) > 1:
                        lc[1] = "'[' || " + lc[1] + " || ']'"
                    if len(lc) > 2:
                        lc[2] = "'[' || " + lc[2] + " || ']'"
                    if len(lc) > 3:
                        lc[3] = "'[' || " + lc[3] + " || ']'"
                    if params.get('labelhint', '') == 'replace-multiline':
                        c = " || E'\\n' || ".join(lc)
                    else:
                        c = " || ' ' || ".join(lc)
                    params['table'] = re.sub(re.compile('(name|\(?coalesce.*) as name', re.DOTALL), "(" + c + ") as name", params['table'])
                    mq_logging.info("QUERY: layer='%s' labelhint='%s' table='%s'" % (layer.name, params.get('labelhint', ''), params['table']))
                    layer.datasource = CreateDatasource(params)

    def process(self, tile):
        #from lat,lng bbox to mapnik bbox
        p0 = self.map_proj.forward(Coord(tile.bbox[0][1],tile.bbox[0][0]))
        p1 = self.map_proj.forward(Coord(tile.bbox[1][1],tile.bbox[1][0]))
        bbox = Box2d(p0,p1)        
        image = Image(tile.size[0],tile.size[1])
        features = None
        
        result = self._check_region(bbox)
        if result is not None:
            if result[0]:
                result[1].resize(image.width(),image.height())
                result[1].zoom_to_box(bbox)
                self.adjust_language(result(1), tile.lang)
                render(result[1],image)
                features = self.save_rendered_metadata(result[1], tile.size, tile.dimensions)
            else :
                mq_logging.info("COMPOSITE MAP: %s" % result[2])
                default_image = Image(tile.size[0],tile.size[1])
                # mask style
                self.mask_map.resize(image.width(),image.height())
                self.mask_map.zoom_to_box(bbox)
                self.adjust_language(self.mask_map, tile.lang)
                render(self.mask_map,image)
                
                # default style
                self.default_map.resize(default_image.width(),default_image.height())
                self.default_map.zoom_to_box(bbox)
                self.adjust_language(self.default_map, tile.lang)
                render(self.default_map,default_image)
                features = self.save_rendered_metadata(self.default_map, tile.size, tile.dimensions)
                
                # composite DST_OUT
                default_image.composite(image,CompositeOp.dst_out)
                
                # current style
                result[1].resize(image.width(),image.height())
                result[1].zoom_to_box(bbox)
                image.set_alpha(0)
                self.adjust_language(result[1], tile.lang)
                render(result[1],image)
                if features is not None:
                    features.features.extend(self.save_rendered_metadata(result[1], tile.size, tile.dimensions).features)
                else:
                    features = self.save_rendered_metadata(result[1], tile.size, tile.dimensions)

                # blend 
                image.blend(0,0,default_image,1.0)                
        else :
            # use default style
            self.default_map.resize(image.width(),image.height())
            self.default_map.zoom_to_box(bbox)
            self.adjust_language(self.default_map, tile.lang)
            render(self.default_map,image)
            features = self.save_rendered_metadata(self.default_map, tile.size, tile.dimensions)

        #convert to PIL image
        image = PIL.Image.frombuffer('RGBA', (image.width(), image.height()), image.tostring(), 'raw', 'RGBA', 0, 3)

        return RenderResult.from_image(tile, image, features)
