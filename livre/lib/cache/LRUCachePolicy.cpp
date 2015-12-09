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

#include <livre/core/cache/Cache.h>
#include <livre/core/cache/CacheObject.h>

#include <livre/lib/cache/LRUCachePolicy.h>

namespace livre
{

struct LastUsedOrderOperator
{
    bool operator()( const CacheObject* object1, const CacheObject* object2 )
    {
        return  object1->getLastUsed( ) < object2->getLastUsed( );
    }
};

LRUCachePolicy::LRUCachePolicy( const size_t maxMem )
    : maxMemoryInBytes_( maxMem ),
      cleanUpRatio_( 1.0 )
{}

void LRUCachePolicy::setCleanupRatio( float cleanUpRatio )
{
   cleanUpRatio_ = cleanUpRatio;
}

bool LRUCachePolicy::willPolicyBeActivated( const Cache& cache ) const
{
    const size_t usedMemoryInBytes = cache.getStatistics().getUsedMemory();
    return usedMemoryInBytes >= maxMemoryInBytes_;
}

bool LRUCachePolicy::isPolicySatisfied( const Cache& cache ) const
{
    const size_t usedMemoryInBytes = cache.getStatistics().getUsedMemory();
    return usedMemoryInBytes < ( 1.0f - cleanUpRatio_ ) * maxMemoryInBytes_;
}

void LRUCachePolicy::apply_( const Cache& cache LB_UNUSED,
                             const std::vector< CacheObject * >& cacheObjectList,
                             std::vector< CacheObject * >& modifiedObjectList )
{
    modifiedObjectList.reserve( cacheObjectList.size() );

    for( std::vector< CacheObject *>::const_iterator it = cacheObjectList.begin();
         it != cacheObjectList.end(); ++it )
    {
        CacheObject* cacheObject = *it;
        const CacheIdSet::const_iterator& itCacheObject =
                protectUnloadingList_.find( cacheObject->getCacheId() );
        if( itCacheObject == protectUnloadingList_.end() )
            modifiedObjectList.push_back( cacheObject );
    }

    std::sort( modifiedObjectList.begin( ), modifiedObjectList.end( ), LastUsedOrderOperator() );

}


}
