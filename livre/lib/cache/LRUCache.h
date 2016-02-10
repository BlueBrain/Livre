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

#ifndef _LRUCache_h_
#define _LRUCache_h_

#include <livre/lib/api.h>
#include <livre/core/cache/Cache.h>
#include <livre/lib/cache/LRUCachePolicy.h>

namespace livre
{

/**
 * The LRUCache class, implementation of the \see Cache with Least Recently Used policy to unload nodes. The
 * derived class should implement the abstract method(s) of \see Cache.
 */
class LRUCache : public Cache
{
protected:

    /**
     * @param maxMemBytes Maximum memory
     * @param cleanUpRatio When cache cleanup is triggered, ratio to be cleaned.
     */
    LIVRE_API LRUCache( const size_t maxMemBytes, float cleanUpRatio = 1.0f );

private:
    LIVRE_API void _onLoaded( const CacheObject& cacheObject ) final;
    LRUCachePolicy _cachePolicy;
};

}

#endif // _LRUCache_h_
