/*------------------------------------------------------------------------------
 *
 *  This file is part of rendermq  
 *
 *  Author: jochen@topf.org
 *
 *  Copyright 2012 Jochen Topf
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *-----------------------------------------------------------------------------*/

#ifndef TILE_PATH_PARSER_HPP
#define TILE_PATH_PARSER_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <boost/xpressive/xpressive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility.hpp>
#include <boost/foreach.hpp>

#include "tile_protocol.hpp"
#include "tile_utils.hpp"

namespace rendermq {

/**
 * Class to match URL paths to a given template. To use this initialize it
 * with a template and then use the operator() to match against a URL path.
 *
 * It can be a bit confusing to see whats going on here, because we use
 * regular expressions several times. First we turn a template string like
 * "/tiles/1.0.0/{STYLE}/{Z}/{X}/{Y}.{FORMAT}" into a regular expression by
 * using a regular expression matching the {SOMETHING} bits and replacing
 * them with named captures "(?P<something>...)". This is done once when
 * the tile_path_parser object is intialized.
 * Later we match the URL path for each incoming request against this
 * regular expression to parse out its parameters.
 */
class tile_path_parser : boost::noncopyable {

   /**
    * This struct is used to translate a URL path template into a regular
    * expression thats than later used to match the URL path.
    */
   struct url_pattern_formatter {

      const std::string prefix;
      const std::string suffix;

      // map that will contain parameter to string with regex mapping
      typedef std::map<std::string, std::string> param_map_t;
      param_map_t params;

      std::vector<std::string>& additional_params;

      url_pattern_formatter(std::vector<std::string>& ap) :
         prefix("(?P<"),
         suffix(">[-A-Za-z0-9_,|]*)"),
         params(),
         additional_params(ap) {
            params["style"]  = "(?P<style>[A-Za-z0-9_]+)"; // map style
            params["z"]      = "(?P<z>[12]?[0-9])"; // zoom level
            params["x"]      = "(?P<x>[0-9]{1,7})"; // x coordinate with 1 to 7 digits
            params["y"]      = "(?P<y>[0-9]{1,7})"; // x coordinate with 1 to 7 digits
            params["format"] = "(?P<format>(png|jpg|jpeg|gif|json))"; // map image format
      }

      template<typename Out>
      Out operator()(const boost::xpressive::smatch& what, Out out) const {
         std::string name = what.str(1);
         std::transform(name.begin(), name.end(), name.begin(), ::tolower);
         param_map_t::const_iterator where = params.find(name);
         if (where == params.end()) { // default
            additional_params.push_back(name);
            out = std::copy(prefix.begin(), prefix.end(), out);
            out = std::copy(name.begin(), name.end(), out);
            out = std::copy(suffix.begin(), suffix.end(), out);
         } else { // pre-defined parameters
            const std::string& sub = where->second;
            out = std::copy(sub.begin(), sub.end(), out);
         }
         return out;
      }

   };

   // Regular expression built from the path template.
   boost::xpressive::sregex path_regex;

   // List of additional parameters generated from this path above the basic parameters STYLE, Z, X, and Y
   std::vector<std::string> additional_params;

public:

   /**
    * Initialize tile_path_parser with a path_template. The path template looks
    * like this:
    * "/some/thing/{STYLE}/{PARAM}/{Z}/{X}/{Y}.{FORMAT}"
    *
    * Parameter names can only contain ASCII letters, digits and underscore.
    * They are changed to lower case internally, so it doesn't matter
    * what you use. Parameter names STYLE, X, Y, Z are special and will
    * probably always be needed, but you can also add any other parameters
    * you might need such as LANG for a language choice or so.
    *
    * The optional commands "/status" and "/dirty" are always allowed at
    * the end and should not be part of your template.
    */
   tile_path_parser(const std::string& path_template) {
      const boost::xpressive::sregex template_chars_regex = boost::xpressive::sregex::compile("([.?*+|()^$])"); // matches special regex characters
      const boost::xpressive::sregex template_param_regex = boost::xpressive::sregex::compile("{\\s*([A-Za-z0-9_]+)\\s*}"); // matches template parameter names in {}
      const url_pattern_formatter formatter(additional_params);

      // escape special regex characters in template path
      const std::string path_escaped_special = boost::xpressive::regex_replace(path_template, template_chars_regex, "\\$1");

      // replace {PARAM} template parameters with regex named captures
      std::string path_regex_string = boost::xpressive::regex_replace(path_escaped_special, template_param_regex, formatter);

      // add capture for optional command suffix
      path_regex_string += "(/(?P<COMMAND>(status|dirty)))?";

      LOG_DEBUG(boost::format("Build path regex '%1%' from template '%2%'.") % path_regex_string % path_template);

      // compile and remember final regex
      path_regex = boost::xpressive::sregex::compile(path_regex_string);
   }

   /**
   * Match given URL path. Returns true if there is a match, false otherwise.
   * Sets all matched parameters in the Results class.
   *
   * Results is a template parameter so it can be easily mocked up for tests,
   * usually you would use an object of class tile_protocol.
   */
   template <class Results>
   bool operator()(Results& results, const std::string& path) {
      boost::xpressive::smatch match_results;

      if (!boost::xpressive::regex_match(path, match_results, path_regex)) {
         return false;
      }

      results.x = boost::lexical_cast<int>(match_results["x"]);
      results.y = boost::lexical_cast<int>(match_results["y"]);
      results.z = boost::lexical_cast<int>(match_results["z"]);
      results.style = match_results["style"];

      const std::string format = match_results["format"].str();
      if (format == "png") {
         results.format = fmtPNG;
      } else if (format == "jpg" || format == "jpeg") {
         results.format = fmtJPEG;
      } else if (format == "json") {
         results.format = fmtJSON;
      } else if (format == "gif") {
         results.format = fmtGIF;
      }

      const std::string command = match_results["COMMAND"].str();
      if (command == "status") {
         results.status = cmdStatus;
      } else if (command == "dirty") {
         results.status = cmdDirty;
      } else {
         results.status = cmdRender;
      }

      BOOST_FOREACH(const std::string& ap, additional_params) {
         results.parameters[ap] = match_results[ap];
      }

      return true;
   }

}; // class tile_path_parser

} // namespace rendermq

#endif // TILE_PATH_PARSER_HPP
