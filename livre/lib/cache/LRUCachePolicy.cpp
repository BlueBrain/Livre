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
    bool operator()( const CacheObject* obj1, const CacheObject* obj2 )
    {
        return  obj1->getLastUsed( ) < obj2->getLastUsed( );
    }
};

LRUCachePolicy::LRUCachePolicy()
    : _maxMemBytes( 0 ),
      _cleanUpRatio( 1.0 )
{}

void LRUCachePolicy::setMaximumMemory( const size_t maxMemBytes )
{
    _maxMemBytes = maxMemBytes;
}

void LRUCachePolicy::setCleanupRatio( float cleanUpRatio )
{
   _cleanUpRatio = cleanUpRatio;
}

bool LRUCachePolicy::willPolicyBeActivated( const Cache& cache ) const
{
    const size_t usedMemoryInBytes = cache.getStatistics().getUsedMemory();
    return usedMemoryInBytes >= _maxMemBytes;
}

bool LRUCachePolicy::isPolicySatisfied( const Cache& cache ) const
{
    const size_t usedMemoryInBytes = cache.getStatistics().getUsedMemory();
    return usedMemoryInBytes < ( 1.0f - _cleanUpRatio ) * _maxMemBytes;
}

void LRUCachePolicy::_apply( const Cache& cache LB_UNUSED,
                             const std::vector< CacheObject * >& cacheObjects,
                             std::vector< CacheObject * >& modifiedObjects )
{
    modifiedObjects = cacheObjects;
    std::sort( modifiedObjects.begin(),
               modifiedObjects.end(),
               LastUsedOrderOperator( ));

}


}
