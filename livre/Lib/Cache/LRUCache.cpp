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

#include <livre/Lib/Cache/LRUCache.h>

namespace livre
{

LRUCache::LRUCache()
{
}

void LRUCache::onLoaded_( const CacheObject &cacheObject LB_UNUSED )
{
    applyPolicy( cachePolicy_ );
}

void LRUCache::setProtectList( const CacheIdSet& protectUnloadingList )
{
    cachePolicy_.setProtectList( protectUnloadingList );
}

void LRUCache::clearProtectList( )
{
    cachePolicy_.setProtectList( CacheIdSet() );
}

void LRUCache::setMaximumMemory( const uint32_t maxMemoryInMBytes )
{
    cachePolicy_.setMaximumMemory( maxMemoryInMBytes );
    getStatistics().setMaximumMemory( maxMemoryInMBytes );
}

void LRUCache::setCleanupRatio( float cleanUpRatio )
{
   cachePolicy_.setCleanupRatio( cleanUpRatio );
}

}
