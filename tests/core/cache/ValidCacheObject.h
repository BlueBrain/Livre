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
        , _isLoaded_( false )
    { }

    size_t getSize( ) const final { return CACHE_SIZE; }

    /**
     * @param observer is added to list of observers.
     */
    void registerObserver( livre::CacheObjectObserver* observer )
        { _registerObserver( observer ); }

    /**
     * @param observer is removed from list of observer
     */
    void unregisterObserver( livre::CacheObjectObserver* observer )
        { _unregisterObserver( observer ); }

private:

    bool _load() final
    {
        _isLoaded_ = true;
        return true;
    }

    void _unload() final
    {
        _isLoaded_ = false;
    }

    bool _isLoaded() const final
    {
        return _isLoaded_;
    }

    bool _isLoaded_;
};

typedef boost::shared_ptr< ValidCacheObject > ValidCacheObjectPtr;
typedef boost::shared_ptr< const ValidCacheObject > ConstValidCacheObjectPtr;

}

#endif // _ValidCacheObject_h_
