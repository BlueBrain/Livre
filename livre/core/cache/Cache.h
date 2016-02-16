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
#include <livre/core/cache/CacheObjectObserver.h>

namespace livre
{

/**
 * The Cache class manages the \see CacheObject s according to applied policies, methods
 * are thread safe inserting/querying nodes.
 */
class Cache : public CacheObjectObserver
{
public:

    /**
     * The ApplyResult enum is the result of cache policy application on cache.
     */
    enum ApplyResult
    {
        AR_NOTACTIVATED, //!< The cache policy is not activated.
        AR_ACTIVATED, //!< The cache policy successfully run on cache.
        AR_CACHEBUSY, //!< Cache is being modified.
        AR_EMPTY //!< There is no cache object to unload.
    };

    /**
     * @param cacheObjectID The object cache id to be queried.
     * @return The cache object from cache, if object is not in the list it is created with given cache id.
     */
    LIVRECORE_API CacheObjectPtr get( const CacheId& cacheObjectID );

    /**
     * @param cacheObjectID The object cache id to be queried.
     * @return The cache object from cache, if object is not in the list an empty cache
     * object is returned.
     */
    LIVRECORE_API CacheObjectPtr get( const CacheId& cacheObjectID ) const;

    /**
     * Applies a policy to the cache.
     * @param cachePolicy The policy to be applied to cache.
     * @return The state for the cache policy application.
     */
    LIVRECORE_API ApplyResult applyPolicy( CachePolicy& cachePolicy ) const;

    /**
     * @return The number of cache objects managed ( not the number of loaded objects ).
     */
    LIVRECORE_API size_t getCount() const;

    /**
     * @return Statistics.
     */
    LIVRECORE_API CacheStatistics& getStatistics();

    /**
     * @return Statistics.
     */
    LIVRECORE_API const CacheStatistics& getStatistics() const;

protected:
    LIVRECORE_API Cache();

    LIVRECORE_API virtual ~Cache( );

    /**
     * @param cacheObjectID The object cache id to be queried.
     * @return The cache object from cache, if object is not in the list it is created.
     */
    LIVRECORE_API CacheObjectPtr _get( const CacheId& cacheId );

    /**
     * @param cacheObjectID The object cache id to be queried.
     * @return The cache object from cache, if object is not in the list an invalid cache object is returned.
     */
    LIVRECORE_API CacheObjectPtr _get( const CacheId& cacheId ) const;

    /**
     * @param cacheID The derived class allocates an \see CacheObject with the given ID
     * @return The allocated object.
     */
    virtual CacheObject* _generate( const CacheId& cacheID ) = 0;

    /**
     * @return Enforce unloading all objects. This function is convinient when
     * object has references to the parent cache and on cache destruction time
     * objects want to use their parent caches.
     */
    LIVRECORE_API void _unloadAll();

    CacheStatisticsPtr _statistics;  //!< The statistics object ptr.

private:

    void _unload( CachePolicy& cachePolicy,
                  const std::vector< CacheObject* >& cacheObjects ) const;

    CacheMap _cacheMap;
    mutable ReadWriteMutex _mutex;
};

}

#endif // _Cache_h_
