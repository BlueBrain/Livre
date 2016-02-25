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

#ifndef _Cache_h_
#define _Cache_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/lunchboxTypes.h>

namespace livre
{

/**
 * The Cache class manages the \see CacheObject s according to LRU Policy, methods
 * are thread safe inserting/querying nodes.
 */
class Cache
{
public:

    /**
     * @param name is the name of the cache.
     * @param maxMemBytes maximum memory.
     */
    LIVRECORE_API explicit Cache( const std::string& name,
                                  size_t maxMemBytes );
    LIVRECORE_API virtual ~Cache();

    /**
     * @param cacheId The object cache id to be queried.
     * @return The cache object from cache, if object is not in the list an empty cache
     * object is returned.
     */
    LIVRECORE_API ConstCacheObjectPtr get( const CacheId& cacheId ) const;

    /**
     * Loads the object to cache. If object is not in the cache it is created.
     * @param cacheId The object cache id to be loaded.
     * @return the loaded or previously loaded cache object. Return empty pointer
     * if cache id is invalid
     */
    LIVRECORE_API CacheObjectPtr load( const CacheId& cacheId );

    /**
     * Unloads the object from the memory, if there are not any references. The
     * objects are removed from cache
     * @param cacheId The object cache id to be unloaded.
     * @return false if object is not unloaded or cacheId is invalid
     */
    LIVRECORE_API bool unload( const CacheId& cacheId );

    /**
     * @return The number of cache objects managed.
     */
    LIVRECORE_API size_t getCount() const;

    /**
     * @return Statistics.
     */
    LIVRECORE_API const CacheStatistics& getStatistics() const;

protected:

    /**
     * @param cacheId The derived class allocates an \see CacheObject with the given ID
     * @return The allocated object.
     */
    virtual CacheObject* _generate( const CacheId& cacheId ) = 0;

    /**
     * @return Enforce unloading all objects. This function is convinient when
     * object has references to the parent cache and on cache destruction time
     * objects want to use their parent caches.
     */
    LIVRECORE_API void _unloadAll();

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _Cache_h_
