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

#include <cstdlib>
#include <lunchbox/scopedMutex.h>

#include <livre/core/cache/CacheObject.h>
#include <livre/core/cache/CacheObjectListener.h>
#include <livre/core/cache/CacheObjectObserver.h>
#include <livre/core/cache/CacheStatistics.h>

namespace livre
{

struct CacheObject::Status : public CacheObjectListener
{
    Status( const CacheId& cacheId_ ) :
        nRef( 0 ),
        lastUsedTime( 0.0 ),
        loadTime( 0.0 ),
        unloadable( true ),
        cacheId( cacheId_ )
    { }

    uint32_t nRef;
    double lastUsedTime;
    double loadTime;
    bool unloadable;
    CacheId cacheId;
    ReadWriteMutex mutex;
};

CacheObject::CacheObject( const CacheId& cacheId )
    : _status( new Status( cacheId ) )
{
}

void CacheObject::_increaseReference( )
{
    WriteLock lock( _status->mutex );
    _status->nRef++;

    for( CacheObjectObserverSet::iterator it = _status->getObservers().begin();
         it != _status->getObservers().end();
         ++it )
    {
        (*it)->_onReferenced( *this );
    }
}

void CacheObject::_decreaseReference( )
{
    WriteLock lock( _status->mutex );
    _status->nRef--;

    if( _status->nRef == 0 && _isValid() )
    {
        for( CacheObjectObserverSet::iterator it = _status->getObservers().begin();
             it != _status->getObservers().end();
             ++it )
        {
            (*it)->_onUnload( *this );
        }
        _unload();
    }

    for( CacheObjectObserverSet::iterator it = _status->getObservers().begin();
         it != _status->getObservers().end();
         ++it )
    {
        (*it)->_onUnreferenced( *this );
    }
}

bool CacheObject::isLoaded() const
{
    ReadLock lock( _status->mutex );
    const bool ret = _isValid( ) && _isLoaded( );
    for( CacheObjectObserverSet::const_iterator it = _status->getObservers().begin();
         it != _status->getObservers().end();
         ++it )
    {
        ret ? (*it)->_onCacheHit( *this ) : (*it)->_onCacheMiss( *this );
    }
    return ret;
}

bool CacheObject::isValid() const
{
    return ( _status ? _isValid() : false );
}

CacheId CacheObject::getId() const
{
    return _status->cacheId;
}

void CacheObject::load()
{
    WriteLock lock( _status->mutex );
    if( _isLoaded( ) )
        return;

    const float start = ThreadClock::getClock().getTimef( );
    if( !_load( ))
        return;

    _status->loadTime = ThreadClock::getClock().getTimef() - start;

    for( CacheObjectObserverSet::iterator it = _status->getObservers().begin();
         it != _status->getObservers().end();
         ++it )
    {
        (*it)->_onLoaded( *this );
    }
}

void CacheObject::unload( )
{
    WriteLock lock( _status->mutex, boost::try_to_lock );
    if( !lock.owns_lock() )
        return;

    if( !isUnloadable() )
        return;

    if( !_isLoaded( ) || _status->nRef > 1 )
    {
        return;
    }

    for( CacheObjectObserverSet::iterator it = _status->getObservers().begin();
         it != _status->getObservers().end();
         ++it )
    {
        (*it)->_onUnload( *this );
    }

    _unload( );
    _status->lastUsedTime = 0;
}

double CacheObject::getLastUsed() const
{
    return _status->lastUsedTime;
}

double CacheObject::getLoadTime() const
{
    return _status->loadTime;
}

bool CacheObject::isUnloadable() const
{
    return _status->unloadable;
}

void CacheObject::setUnloadable( bool unloadable )
{
    _status->unloadable = unloadable;
}

uint32_t CacheObject::getReferenceCount_( ) const
{
    return _status->nRef;
}

void CacheObject::registerObserver( CacheObjectObserver* observer )
{
    _status->registerObserver( observer );
}

void CacheObject::unregisterObserver( CacheObjectObserver* observer )
{
    _status->unregisterObserver( observer );
}

bool CacheObject::_isValid() const
{
    return _status->cacheId != INVALID_CACHE_ID;
}

bool CacheObject::operator==( const CacheObject& cacheObject ) const
{
    return _status->cacheId == cacheObject.getId();
}

void CacheObject::updateLastUsedWithCurrentTime_()
{
     _status->lastUsedTime = ThreadClock::getClock().getTimef();
}

}
