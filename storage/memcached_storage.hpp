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

#ifndef RENDERMQ_MEMCACHED_STORAGE_HPP
#define RENDERMQ_MEMCACHED_STORAGE_HPP

#include <string>
#include <ctime>
#include "tile_storage.hpp"

class memcached_st;

namespace rendermq {

/*
 *  Metatile storage in memcached.
 *
 *  Metatiles are stored in memcached in the same format used elsewhere.
 *
 *  No creation or expire timestamp is stored. Storage in memcached is
 *  for short-time only, so this should not be needed.
 */
class memcached_storage : public tile_storage {
  std::string key_string(const tile_protocol &tile) const;
public:
  class handle : public tile_storage::handle {
  public:
    handle(const std::pair<metatile_reader::iterator_type, metatile_reader::iterator_type>&);
    virtual ~handle();
    virtual bool exists() const;
    virtual std::time_t last_modified() const;
    virtual bool data(std::string &) const;
    virtual bool expired() const;
  private:
    std::string tile_data;
  };
  friend class handle;

  memcached_storage(const std::string& options);
  virtual ~memcached_storage();

  boost::shared_ptr<tile_storage::handle> get(const tile_protocol &tile) const;
  bool get_meta(const tile_protocol &tile, std::string &) const;
  bool put_meta(const tile_protocol &tile, const std::string &buf) const;
  bool expire(const tile_protocol &tile) const;
private:
  memcached_st* memcache;
};

}

#endif // RENDERMQ_MEMCACHED_STORAGE_HPP
