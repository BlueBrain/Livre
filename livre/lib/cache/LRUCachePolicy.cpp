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

LRUCachePolicy::LRUCachePolicy( size_t maxMemBytes,
                                float cleanUpRatio /* = 1.0f */ )
    : _maxMemBytes( maxMemBytes )
    , _cleanUpRatio( cleanUpRatio )
{}

bool LRUCachePolicy::willPolicyBeActivated( const Cache& cache ) const
{
    const size_t usedMemBytes = cache.getStatistics().getUsedMemory();
    return usedMemBytes >= _maxMemBytes;
}

bool LRUCachePolicy::isPolicySatisfied( const Cache& cache ) const
{
    const size_t usedMemBytes = cache.getStatistics().getUsedMemory();
    return usedMemBytes < ( 1.0f - _cleanUpRatio ) * _maxMemBytes;
}

void LRUCachePolicy::_apply( const Cache& cache LB_UNUSED,
                             const std::vector< CacheObject* >& cacheObjects,
                             std::vector< CacheObject* >& modifiedObjects )
{
    modifiedObjects = cacheObjects;
    std::sort( modifiedObjects.begin( ), modifiedObjects.end( ), LastUsedOrderOperator() );

}


}
