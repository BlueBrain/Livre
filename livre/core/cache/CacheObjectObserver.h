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

#ifndef _CacheObjectObserver_h_
#define _CacheObjectObserver_h_

#include <lunchbox/compiler.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * The CacheObjectObserver class help \see CacheObject to notify its actions.
 */
class CacheObjectObserver
{
    friend class CacheObject;

protected:
    /**
     * Notifies the CacheObserver after loading.
     * @param cacheObject The CacheObject that notifies.
     */
    virtual void onLoaded_( const CacheObject& cacheObject LB_UNUSED ) { }

    /**
     * Notifies the CacheObserver before unloading.
     * @param cacheObject The CacheObject that notifies.
     */
    virtual void onUnload_( const CacheObject& cacheObject LB_UNUSED ) { }

    /**
     * Notifies the CacheObserver on increase of reference count.
     * @param cacheObject The CacheObject that notifies.
     */
    virtual void onReferenced_( const CacheObject& cacheObject LB_UNUSED ) { }

    /**
     * Notifies the CacheObserver on decrease of reference count.
     * @param cacheObject The CacheObject that notifies.
     */
    virtual void onUnreferenced_( const CacheObject& cacheObject LB_UNUSED ) { }

    /**
     * Notifies the CacheObserver if object is in Cache
     * @param cacheObject The CacheObject that notifies.
     */
    virtual void onCacheHit_( const CacheObject& cacheObject LB_UNUSED ) { }

    /**
     * Notifies notify the CacheObserver if object is not in Cache
     * @param cacheObject The CacheObject that notifies.
     */
    virtual void onCacheMiss_( const CacheObject& cacheObject LB_UNUSED ) { }

    virtual ~CacheObjectObserver() {}
};

}

#endif // _CacheObjectObserver_h_
