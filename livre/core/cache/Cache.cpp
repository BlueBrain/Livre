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
#include <livre/core/cache/Cache.h>
#include <livre/core/cache/CacheObject.h>
#include <livre/core/cache/CachePolicy.h>
#include <livre/core/cache/CacheStatistics.h>

#define CACHE_LOG_SIZE 1000000

namespace livre
{

CacheObjectPtr Cache::get( const CacheId& cacheId )
{
    return _get( cacheId );
}

CacheObjectPtr Cache::get( const CacheId& cacheId ) const
{
    return _get( cacheId );
}

Cache::ApplyResult Cache::applyPolicy( CachePolicy& cachePolicy ) const
{
    ReadLock readLock( _mutex, boost::try_to_lock );
    if( !readLock.owns_lock() )
        return AR_CACHEBUSY;

    if( _cacheMap.empty() || !cachePolicy.willPolicyBeActivated( *this ) )
        return AR_NOTACTIVATED;

    std::vector< CacheObject* > cacheObjects;
    cacheObjects.reserve( _cacheMap.size() );

    for( CacheMap::const_iterator it = _cacheMap.begin(); it != _cacheMap.end(); ++it )
    {
        const CacheObjectPtr& object = it->second;
        if( object && object->isValid() && object->_isLoaded() )
            cacheObjects.push_back( object.get( ));
    }

    if( cacheObjects.empty( ))
        return AR_EMPTY;

    std::vector< CacheObject * > modifiedList;
    cachePolicy._apply( *this, cacheObjects, modifiedList );

    _unload( cachePolicy, modifiedList );
    return AR_ACTIVATED;
}

Cache::Cache()
    : _statistics( new CacheStatistics( "Statistics", CACHE_LOG_SIZE ) )
{
}

Cache::~Cache()
{
    for( CacheMap::iterator it = _cacheMap.begin(); it != _cacheMap.end(); ++it )
    {
        CacheObjectPtr& cacheObject = it->second;
        cacheObject->_unregisterObserver( this );
        cacheObject->_unregisterObserver( _statistics.get( ));
    }
}

CacheObjectPtr Cache::_get( const CacheId& cacheId )
{
    if( cacheId == INVALID_CACHE_ID )
        return CacheObjectPtr();

    WriteLock writeLock( _mutex );
    CacheMap::iterator it = _cacheMap.find( cacheId );
    if( it == _cacheMap.end() )
    {
        CacheObjectPtr cacheObject( _generate( cacheId ));
        cacheObject->_registerObserver( this );
        cacheObject->_registerObserver( _statistics.get() );
        _cacheMap[ cacheId ] = cacheObject;
    }

    LBASSERT( _cacheMap[ cacheId ]->_status );

    return _cacheMap[ cacheId ];
}

CacheObjectPtr Cache::_get( const CacheId& cacheId ) const
{
    ReadLock readLock( _mutex );
    CacheMap::const_iterator it = _cacheMap.find( cacheId );

    if( it == _cacheMap.end() )
        return CacheObjectPtr();

    LBASSERT( it->second->_status );

    return it->second;
}

void Cache::_unloadAll()
{
    for( CacheMap::iterator it = _cacheMap.begin(); it != _cacheMap.end(); ++it )
        it->second->_unload();
}

void Cache::_unload( CachePolicy& cachePolicy,
                     const std::vector< CacheObject* >& cacheObjects ) const
{
    for( std::vector< CacheObject *>::const_iterator it = cacheObjects.begin();
         it != cacheObjects.end(); ++it )
    {
        ( *it )->unload();
        if( cachePolicy.isPolicySatisfied( *this ))
            return;
    }
}

size_t Cache::getCount() const
{
    ReadLock readLock( _mutex );
    return _cacheMap.size();
}

CacheStatistics& Cache::getStatistics()
{
    return *_statistics;
}

const CacheStatistics& Cache::getStatistics() const
{
    return *_statistics;
}

}
