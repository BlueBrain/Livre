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

#ifndef _EmptyCacheObject_h_
#define _EmptyCacheObject_h_

#include <livre/core/Data/LODNode.h>
#include <livre/core/Cache/CacheObject.h>

namespace livre
{

/**
 * The EmptyCacheObject class for an invalid, empty \see CacheObject.
 */
class EmptyCacheObject : public CacheObject
{
public:

    /**
     * @return The empty \see CacheObject.
     */
    static CacheObject* getEmptyPtr()
    {
        static boost::shared_ptr< EmptyCacheObject > data( new EmptyCacheObject() );
        return data.get();
    }

    /**
     * @return The invalid cache id.
     */
    virtual CacheId getCacheID() const { return INVALID_CACHE_ID; }

    /**
     * @return The empty size.
     */
    virtual uint32_t getCacheSize( ) const { return 0; }

private:

    EmptyCacheObject( ) { }
    virtual bool load_( ) { return false; }
    virtual void unload_( ) { }
    virtual bool isValid_( ) const { return false; }
    virtual bool isLoaded_( ) const { return false; }
};

}

#endif // _EmptyCacheObject_h_
