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

#ifndef _LRUCache_h_
#define _LRUCache_h_

#include <livre/lib/api.h>
#include <livre/core/cache/Cache.h>
#include <livre/lib/cache/LRUCachePolicy.h>

namespace livre
{

/**
 * The LRUCache class, implementation of the \see Cache with Least Recently Used policy to unload nodes. The
 * derived class should implement the abstract method(s) of \see Cache.
 */
class LRUCache : public Cache
{
public:
    /**
     * Sets a list of node ids to be protected from unloading.
     * @param protectUnloadingList The set of node ids.
     */
    LIVRE_API void setProtectList( const CacheIdSet& protectUnloadingList );

    /**
     * Clears the list of node ids to be protected.
     */
    LIVRE_API void clearProtectList( );

    /**
     * Sets the maximum memory.
     * @param maxMemoryInBytes Maximum memory in bytes.
     */
    LIVRE_API void setMaximumMemory( const size_t maxMemoryInBytes );

    /**
     * Sets the clean up ratio.
     * @param cleanUpRatio Once the policy is activated, ( 1.0 - cleanUpRatio ) can be cleaned.
     */
    LIVRE_API void setCleanupRatio( float cleanUpRatio );

protected:
    LIVRE_API LRUCache();

private:
    LIVRE_API void onLoaded_( const CacheObject &cacheObject );
    LRUCachePolicy cachePolicy_;
};

}

#endif // _LRUCache_h_
