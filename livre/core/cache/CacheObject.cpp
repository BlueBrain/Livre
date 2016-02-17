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

namespace livre
{

struct CacheObject::Status
{
    Status( const CacheId& cacheId_ )
        : nRef( 0 )
        , cacheId( cacheId_ )
    {}

    uint32_t nRef;
    CacheId cacheId;
    ReadWriteMutex mutex;
};

CacheObject::CacheObject( const CacheId& cacheId )
    : _status( new Status( cacheId ) )
{
}

void CacheObject::_increaseRef()
{
    WriteLock lock( _status->mutex );
    _status->nRef++;
}

void CacheObject::_decreaseRef()
{
    WriteLock lock( _status->mutex );
    _status->nRef--;
}

bool CacheObject::isLoaded() const
{
    ReadLock lock( _status->mutex );
    const bool ret = _isValid( ) && _isLoaded( );
    return ret;
}

bool CacheObject::isValid() const
{
    return _isValid();
}

CacheId CacheObject::getId() const
{
    return _status->cacheId;
}

bool CacheObject::_cacheLoad()
{
    WriteLock lock( _status->mutex );
    return _load( );
}

bool CacheObject::_cacheUnload()
{
    WriteLock lock( _status->mutex, boost::try_to_lock );
    if( !lock.owns_lock() )
        return false;

    if( _status->nRef > 0 )
        return false;

    _unload( );
    return true;
}

uint32_t CacheObject::getRefCount( ) const
{
    return _status->nRef;
}

bool CacheObject::_isValid() const
{
    return _status->cacheId != INVALID_CACHE_ID;
}

bool CacheObject::operator==( const CacheObject& cacheObject ) const
{
    return _status->cacheId == cacheObject.getId();
}

}
