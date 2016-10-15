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

#ifndef _CacheObject_h_
#define _CacheObject_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/lunchboxTypes.h>

namespace livre
{

/**
 * The CacheObject class for cached objects that can be managed with \see Cache.
 */
class CacheObject
{
public:

    LIVRECORE_API virtual ~CacheObject();

    /** @return The memory size of the object in bytes. */
    virtual size_t getSize() const = 0;

    /** @return The unique cache id. */
    LIVRECORE_API CacheId getId() const;

    /** @return On default returns true if cache ids are same */
    virtual bool operator==( const CacheObject& cacheObject ) const;

protected:

    /**
     * Constructor
     * The inheriting class should throw CacheLoadException if the object can not be constructed
     */
    LIVRECORE_API explicit CacheObject( const CacheId& cacheId = INVALID_CACHE_ID );

private:

    struct Impl;
    std::unique_ptr< Impl > _impl;
};

}

#endif // _CacheObject_h_
