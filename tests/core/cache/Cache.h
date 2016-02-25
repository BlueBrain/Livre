/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *
 * This file is part of Livre <https://github.com/BlueBrain/Livre>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _TestCache_h_
#define _TestCache_h_

#include "ValidCacheObject.h"

#include <livre/core/cache/Cache.h>

namespace test
{

class Cache : public livre::Cache
{
public:
    Cache( size_t maxMemBytes )
        : livre::Cache( "Test Cache" ,maxMemBytes ) {}

private:
    livre::CacheObject* _generate( const livre::CacheId& cacheId )
    {
        ValidCacheObject* cacheObject =
                new test::ValidCacheObject( cacheId );
        return cacheObject;
    }
};

}

#endif // _TestCache_h_
