/*------------------------------------------------------------------------------
 *
 *  This file is part of rendermq  
 *
 *  Author: john.novak@mapquest.com
 *  Author: kevin.kreiser@mapquest.com
 *
 *  Copyright 2010-1 Mapquest, Inc.  All Rights reserved.
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

#ifndef RENDERMQ_HASHWRAPPER_HPP
#define RENDERMQ_HASHWRAPPER_HPP

#include "config.hpp"
#include "meta_tile.hpp"

// hashkit changed their headers at some point to take the C++ stuff and put
// it in a different file. the configure script should detect this and set
// HAVE_LIBHASHKIT_1_0_HASHKIT_HPP appropriately.
#ifdef HAVE_LIBHASHKIT_1_0_HASHKIT_HPP
# include <libhashkit-1.0/hashkit.hpp>
#else
# include <libhashkit/hashkit.h>
#endif

#include <libmemcached/memcached.h>
#include <stdexcept>
#include <vector>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>

using boost::shared_ptr;

namespace rendermq
{ 
   typedef std::vector< std::pair< std::string, int > >		vecHostInfo;
   typedef vecHostInfo::iterator								      iterVecHostInfo;
   typedef vecHostInfo::const_iterator								citerVecHostInfo;
	
   class hashWrapper
   {
      protected:
         memcached_st *pMemc;
         shared_ptr<Hashkit> pHashKit;
         hashkit_hash_algorithm_t hashType;
         unsigned int serverCount;
	
      public:
         //
         hashWrapper(const std::string &config, const vecHostInfo &hosts);
         virtual ~hashWrapper();

         //returns the total number of hashable hosts
         unsigned int getHostCount() const {return serverCount;}
         //given a string to hash on, hash the string to a particular host, optionally return host offset from the hashed host
         std::pair<std::string, int> getHost(const std::string &hashValue, const unsigned int& offset = 0);
         //change the hash type
         void setHashType(hashkit_hash_algorithm_t hashType);
         //get the hash type
         hashkit_hash_algorithm_t getHashType();
			//returns the hash value for a given string
         uint32_t getHash(std::string &hashValue);
	};
}

#endif // RENDERMQ_HASHWRAPPER_HPP
