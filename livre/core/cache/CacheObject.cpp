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
        : cacheId( cacheId_ )
    {}

    CacheId cacheId;
    ReadWriteMutex mutex;
};

CacheObject::CacheObject( const CacheId& cacheId )
    : _status( new Status( cacheId ) )
{
}

CacheObject::~CacheObject()
{
}

bool CacheObject::isValid() const
{
    ReadLock lock( _status->mutex );
    return _isValid();
}

CacheId CacheObject::getId() const
{
    return _status->cacheId;
}

size_t CacheObject::getSize() const
{
    ReadLock lock( _status->mutex );
    return _getSize();
}

bool CacheObject::_isValid() const
{
    return _status->cacheId != INVALID_CACHE_ID;
}

size_t CacheObject::_getSize() const
{
    return 0;
}

bool CacheObject::operator==( const CacheObject& cacheObject ) const
{
    return _status->cacheId == cacheObject.getId();
}

}
