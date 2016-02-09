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

#ifndef _ValidCacheObject_h_
#define _ValidCacheObject_h_

#include <livre/core/cache/CacheObject.h>

namespace test
{

const size_t CACHE_SIZE = 1000;

class ValidCacheObject : public livre::CacheObject
{
public:

    /**
     * Constructor
     */
    ValidCacheObject( const livre::CacheId& cacheId )
        : livre::CacheObject( cacheId )
        , isLoadedVar_( false )
    { }

    size_t getCacheSize( ) const final { return CACHE_SIZE; }

private:


    virtual bool _load( )
    {
        isLoadedVar_ = true;
        return true;
    }

    virtual void _unload( )
    {
        isLoadedVar_ = false;
    }

    virtual bool _isLoaded( ) const
    {
        return isLoadedVar_;
    }

    bool isLoadedVar_;
};

typedef boost::shared_ptr< ValidCacheObject > ValidCacheObjectPtr;
typedef boost::shared_ptr< const ValidCacheObject > ConstValidCacheObjectPtr;

}

#endif // _ValidCacheObject_h_
