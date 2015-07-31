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

#include <livre/core/types.h>
#include <livre/core/lunchboxTypes.h>
#include <livre/core/Cache/CacheObjectObserver.h>

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
    CacheObjectPtr getObjectFromCache( const CacheId cacheObjectID );

    /**
     * Applies a policy to the cache.
     * @param cachePolicy The policy to be applied to cache.
     * @return The state for the cache policy application.
     */
    ApplyResult applyPolicy( CachePolicy& cachePolicy ) const;

    /**
     * @return The number of cache objects managed ( not the number of loaded objects ).
     */
    size_t getNumberOfCacheObjects( ) const;

    /**
     * @return Statistics.
     */
    CacheStatistics& getStatistics( );

    /**
     * @return Statistics.
     */
    const CacheStatistics& getStatistics( ) const;

protected:

    Cache( );

    virtual ~Cache( );

    /**
     * @param cacheObjectID The object cache id to be queried.
     * @return The cache object from cache, if object is not in the list it is created.
     */
    CacheObjectPtr getObjectFromCache_( const CacheId cacheObjectID );

    /**
     * @param cacheObjectID The object cache id to be queried.
     * @return The cache object from cache, if object is not in the list an invalid cache object is returned.
     */
    CacheObjectPtr getObjectFromCache_( const CacheId cacheObjectID ) const;

    /**
     * @param cacheID The derived class allocates an \see CacheObject with the given ID
     * @return The allocated object.
     */
    virtual CacheObject* generateCacheObjectFromID_( const CacheId cacheID ) = 0;

    CacheStatisticsPtr statisticsPtr_;  //!< The statistics object ptr.

private:

    void unloadCacheObjectsWithPolicy_( CachePolicy& cachePolicy,
                                        const std::vector< CacheObject * >& cacheObjectList ) const;

    CacheMap cacheMap_;
    mutable ReadWriteMutex mutex_;
};

}

#endif // _Cache_h_
