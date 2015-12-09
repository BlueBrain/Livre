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

struct CacheObject::CacheInfo : public CacheObjectListener
{
    CacheInfo( const CacheId& cacheId_ ) :
        referenceCount( 0 ),
        lastUsedTime( 0.0 ),
        loadTime( 0.0 ),
        unloadable( true ),
        cacheId( cacheId_ )
    { }

    uint32_t referenceCount;
    double lastUsedTime;
    double loadTime;
    bool unloadable;
    CacheId cacheId;
    ReadWriteMutex mutex;
};

CacheObject::CacheObject( const CacheId& cacheId )
    : commonInfoPtr_( new CacheInfo( cacheId ) )
{
}

void CacheObject::increaseReference_( )
{
    WriteLock lock( commonInfoPtr_->mutex );
    commonInfoPtr_->referenceCount++;

    for( CacheObjectObserverSet::iterator it = commonInfoPtr_->getObservers().begin();
         it != commonInfoPtr_->getObservers().end();
         ++it )
    {
        (*it)->onReferenced_( *this );
    }
}

void CacheObject::decreaseReference_( )
{
    WriteLock lock( commonInfoPtr_->mutex );
    commonInfoPtr_->referenceCount--;

    if( commonInfoPtr_->referenceCount == 0 && isValid_() )
    {
        for( CacheObjectObserverSet::iterator it = commonInfoPtr_->getObservers().begin();
             it != commonInfoPtr_->getObservers().end();
             ++it )
        {
            (*it)->onUnload_( *this );
        }
        unload_();
    }

    for( CacheObjectObserverSet::iterator it = commonInfoPtr_->getObservers().begin();
         it != commonInfoPtr_->getObservers().end();
         ++it )
    {
        (*it)->onUnreferenced_( *this );
    }
}

bool CacheObject::isLoaded() const
{
    ReadLock lock( commonInfoPtr_->mutex );
    const bool ret = isValid_( ) && isLoaded_( );
    for( CacheObjectObserverSet::const_iterator it = commonInfoPtr_->getObservers().begin();
         it != commonInfoPtr_->getObservers().end();
         ++it )
    {
        ret ? (*it)->onCacheHit_( *this ) : (*it)->onCacheMiss_( *this );
    }
    return ret;
}

bool CacheObject::isValid() const
{
    return ( commonInfoPtr_ ? isValid_() : false );
}

CacheId CacheObject::getCacheId() const
{
    return commonInfoPtr_->cacheId;
}

void CacheObject::cacheLoad()
{
    WriteLock lock( commonInfoPtr_->mutex );
    if( isLoaded_( ) )
        return;

    const float start = ThreadClock::getClock().getTimef( );
    if( !load_( ))
        return;

    commonInfoPtr_->loadTime = ThreadClock::getClock().getTimef() - start;

    for( CacheObjectObserverSet::iterator it = commonInfoPtr_->getObservers().begin();
         it != commonInfoPtr_->getObservers().end();
         ++it )
    {
        (*it)->onLoaded_( *this );
    }
}

void CacheObject::cacheUnload( )
{
    WriteLock lock( commonInfoPtr_->mutex, boost::try_to_lock );
    if( !lock.owns_lock() )
        return;

    if( !isUnloadable() )
        return;

    if( !isLoaded_( ) || commonInfoPtr_->referenceCount > 1 )
    {
        return;
    }

    for( CacheObjectObserverSet::iterator it = commonInfoPtr_->getObservers().begin();
         it != commonInfoPtr_->getObservers().end();
         ++it )
    {
        (*it)->onUnload_( *this );
    }

    unload_( );
    resetLastUsed_();
}

double CacheObject::getLastUsed( ) const
{
    return commonInfoPtr_->lastUsedTime;
}

double CacheObject::getLoadTime( ) const
{
    return commonInfoPtr_->loadTime;
}

bool CacheObject::isUnloadable( ) const
{
    return commonInfoPtr_->unloadable;
}

void CacheObject::setUnloadable( bool unloadable )
{
    commonInfoPtr_->unloadable = unloadable;
}

uint32_t CacheObject::getReferenceCount_( ) const
{
    return commonInfoPtr_->referenceCount;
}

void CacheObject::registerObserver( CacheObjectObserver* observer )
{
    commonInfoPtr_->registerObserver( observer );
}

void CacheObject::unregisterObserver( CacheObjectObserver* observer )
{
    commonInfoPtr_->unregisterObserver( observer );
}

bool CacheObject::isValid_() const
{
    return commonInfoPtr_->cacheId != INVALID_CACHE_ID;
}

bool CacheObject::operator==( const CacheObject& cacheObject ) const
{
    return commonInfoPtr_->cacheId == cacheObject.getCacheId();
}

void CacheObject::updateLastUsed_( const double lastUsedTime )
{
    commonInfoPtr_->lastUsedTime = lastUsedTime;
}

void CacheObject::resetLastUsed_( )
{
    updateLastUsed_( 0.0 );
}

void CacheObject::updateLastUsedWithCurrentTime_( )
{
    updateLastUsed_( ThreadClock::getClock().getTimef() );
}

}
