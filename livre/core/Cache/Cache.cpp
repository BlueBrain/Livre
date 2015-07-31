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

#include <livre/core/defines.h>
#include <livre/core/Cache/Cache.h>
#include <livre/core/Cache/CacheObject.h>
#include <livre/core/Cache/CachePolicy.h>
#include <livre/core/Cache/CacheStatistics.h>

#define CACHE_LOG_SIZE 1000000

namespace livre
{

CacheObjectPtr Cache::getObjectFromCache( const CacheId cacheObjectID )
{
    return getObjectFromCache_( cacheObjectID );
}

Cache::ApplyResult Cache::applyPolicy( CachePolicy& cachePolicy ) const
{
    ReadLock readLock( mutex_, boost::try_to_lock );
    if( !readLock.owns_lock() )
        return AR_CACHEBUSY;

    if( cacheMap_.empty() || !cachePolicy.willPolicyBeActivated( *this ) )
        return AR_NOTACTIVATED;

    std::vector< CacheObject* > cacheObjectList;
    cacheObjectList.reserve( cacheMap_.size() );

    for( CacheMap::const_iterator it = cacheMap_.begin(); it != cacheMap_.end(); ++it )
    {
        CacheObject* object = it->second.get();
        if( object && object->isValid() && object->isLoaded_() )
        {
            cacheObjectList.push_back( object );
        }
    }

    if( cacheObjectList.empty() )
        return AR_EMPTY;

    std::vector< CacheObject * > modifiedList;
    cachePolicy.apply_( *this, cacheObjectList, modifiedList );

    unloadCacheObjectsWithPolicy_( cachePolicy, modifiedList );
    return AR_ACTIVATED;
}

Cache::Cache()
    : statisticsPtr_( new CacheStatistics( "Statistics", CACHE_LOG_SIZE ) )
{
}

Cache::~Cache()
{
    for( CacheMap::iterator it = cacheMap_.begin(); it != cacheMap_.end(); ++it )
    {
        CacheObjectPtr cacheObject = it->second;
        cacheObject->unregisterObserver( this );
        cacheObject->unregisterObserver( statisticsPtr_.get() );
    }
}

CacheObjectPtr Cache::getObjectFromCache_( const CacheId cacheObjectID )
{
    LBASSERT( cacheObjectID != INVALID_CACHE_ID );

    WriteLock writeLock( mutex_ );
    CacheMap::iterator it = cacheMap_.find( cacheObjectID );
    if( it == cacheMap_.end() )
    {
        CacheObjectPtr cacheObjectPtr( generateCacheObjectFromID_( cacheObjectID ) );
        cacheObjectPtr->registerObserver( this );
        cacheObjectPtr->registerObserver( statisticsPtr_.get() );
        cacheMap_[ cacheObjectID ] = cacheObjectPtr;
    }

    LBASSERT( cacheMap_[ cacheObjectID ]->commonInfoPtr_ );

    return cacheMap_[ cacheObjectID ];
}

CacheObjectPtr Cache::getObjectFromCache_( const CacheId cacheObjectID ) const
{
    ReadLock readLock( mutex_ );
    CacheMap::const_iterator it = cacheMap_.find( cacheObjectID );

    if( it == cacheMap_.end() )
        return CacheObjectPtr();

    LBASSERT( it->second->commonInfoPtr_ );

    return it->second;
}

void Cache::unloadCacheObjectsWithPolicy_( CachePolicy& cachePolicy,
                                           const std::vector< CacheObject * >& cacheObjectList ) const
{
    for( std::vector< CacheObject *>::const_iterator it = cacheObjectList.begin();
         it != cacheObjectList.end(); ++it )
    {
        ( *it )->cacheUnload( );
        if( cachePolicy.isPolicySatisfied( *this ) )
            return;
    }
}

size_t Cache::getNumberOfCacheObjects( ) const
{
    ReadLock readLock( mutex_ );
    return cacheMap_.size( );
}

CacheStatistics& Cache::getStatistics( )
{
    return *statisticsPtr_;
}

const CacheStatistics& Cache::getStatistics( ) const
{
    return *statisticsPtr_;
}

}
