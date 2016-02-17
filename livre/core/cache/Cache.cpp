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

#include <livre/core/defines.h>
#include <livre/core/cache/Cache.h>
#include <livre/core/cache/CacheObject.h>
#include <livre/core/cache/CacheStatistics.h>

#define CACHE_LOG_SIZE 1000000

namespace livre
{


struct LRUCachePolicy
{

    typedef std::deque< CacheId > LRUQueue;

    LRUCachePolicy( const size_t maxMemBytes,
                    const float cleanupRatio = 1.0f )
        : _maxMemBytes( maxMemBytes )
        , _cleanUpRatio( cleanupRatio )
    {}

    bool isActivated( const Cache& cache ) const
    {
        const size_t usedMemBytes = cache.getStatistics().getUsedMemory();
        return usedMemBytes >= _maxMemBytes;
    }

    bool isSatisfied( const Cache& cache ) const
    {
        const size_t usedMemBytes = cache.getStatistics().getUsedMemory();
        return usedMemBytes < ( 1.0f - _cleanUpRatio ) * _maxMemBytes;
    }

    void insert( const CacheId& cacheId )
    {
        remove( cacheId );
        _lruQueue.push_back( cacheId );
    }

    void remove( const CacheId& cacheId )
    {
        typename LRUQueue::iterator it = _lruQueue.begin();
        while( it != _lruQueue.end( ))
        {
            if( *it == cacheId )
                _lruQueue.erase( it );
            else
                ++it;
        }
    }

    CacheIds getObjects() const
    {
        CacheIds ids;
        ids.reserve( _lruQueue.size( ));
        ids.insert( ids.begin(), _lruQueue.begin(), _lruQueue.end());
        return ids;
    }

    const size_t _maxMemBytes;
    const float _cleanUpRatio;
    LRUQueue _lruQueue;
};

struct Cache::Impl
{
    Impl( Cache& cache,
          const size_t maxMemBytes )
        : _policy( maxMemBytes )
        , _cache( cache )
        , _statistics( "Statistics", CACHE_LOG_SIZE )
    {
    }

    ~Impl()
    {}

    void applyPolicy() const
    {
        if( _cacheMap.empty() || !_policy.isActivated( _cache ) )
            return;

        unload();
    }

    void onDelete( CacheObject* cacheObject ) const
    {
        WriteLock lock( _mutex );
        cacheObject->_decreaseRef();
    }

    CacheObjectPtr getSharedPtr( CacheObject* obj ) const
    {
        obj->_increaseRef();
        return CacheObjectPtr( obj,
                               std::bind( &Impl::onDelete,
                                          this,
                                          std::placeholders::_1 ));
    }

    CacheObjectPtr load( const CacheId& cacheId )
    {
        WriteLock writeLock( _mutex );
        CacheObject* obj = getFromMap( cacheId );
        if( obj->isLoaded( ))
        {
            _statistics._hit();
            _policy.insert( cacheId );
            return getSharedPtr( obj );
        }

        if( obj->_cacheLoad( ))
        {
            _statistics._miss();
            _statistics._loaded( *obj );
            _policy.insert( cacheId );
            applyPolicy();
            return getSharedPtr( obj );
        }
        else
            return CacheObjectPtr();
    }

    bool cacheUnload( const CacheId& cacheId ) const
    {
        CacheObject* obj = getFromMap( cacheId );
        if( obj->_cacheUnload( ))
        {
            _statistics._unloaded( *obj );
            _policy.remove( cacheId );
            return true;
        }
        else
            return false;
    }

    CacheObject* getFromMap( const CacheId& cacheId )
    {
        CacheMap::iterator it = _cacheMap.find( cacheId );
        if( it == _cacheMap.end( ))
        {
            CacheObject* cacheObject( _cache._generate( cacheId ));
            _cacheMap[ cacheId ] = cacheObject;
        }

        return _cacheMap[ cacheId ];
    }

    CacheObject* getFromMap( const CacheId& cacheId ) const
    {
        CacheMap::const_iterator it = _cacheMap.find( cacheId );
        if( it == _cacheMap.end() )
            return nullptr;

        return it->second;
    }

    bool unload( const CacheId& cacheId ) const
    {
        WriteLock lock( _mutex );
        CacheObjectPtr obj = get( cacheId );
        if( !obj || !obj->isLoaded( ))
            return false;

        return cacheUnload( cacheId );
    }

    CacheObjectPtr get( const CacheId& cacheId )
    {
        WriteLock lock( _mutex );
        CacheObject *obj = getFromMap( cacheId );
        return getSharedPtr( obj );
    }

    CacheObjectPtr get( const CacheId& cacheId ) const
    {
        ReadLock lock( _mutex );
        CacheObject *obj = getFromMap( cacheId );
        if( !obj )
            return CacheObjectPtr();
        return getSharedPtr( obj );
    }

    void unloadAll()
    {
        ReadLock lock( _mutex );
        for( CacheMap::iterator it = _cacheMap.begin(); it != _cacheMap.end(); ++it )
            it->second->_unload();
    }

    size_t getCount() const
    {
        ReadLock lock( _mutex );
        return _cacheMap.size();
    }

    void unload() const
    {
        // Objects are returned in delete order
        const CacheIds& cacheIds = _policy.getObjects();
        for( CacheIds::const_iterator it = cacheIds.begin();
             it != cacheIds.end(); ++it )
        {
            cacheUnload( *it );
            if( _policy.isSatisfied( _cache ))
                return;
        }
    }

    mutable LRUCachePolicy _policy;
    Cache& _cache;
    mutable CacheStatistics _statistics;
    CacheMap _cacheMap;
    mutable ReadWriteMutex _mutex;
};

Cache::Cache( const size_t maxMemBytes )
    : _impl( new Cache::Impl( *this, maxMemBytes ))
{
}

Cache::~Cache()
{}

CacheObjectPtr Cache::load( const CacheId& cacheId )
{
    if( cacheId == INVALID_CACHE_ID )
        return CacheObjectPtr();

    return _impl->load( cacheId );
}

bool Cache::unload( const CacheId& cacheId ) const
{
    if( cacheId == INVALID_CACHE_ID )
        return false;

    return _impl->unload( cacheId );
}

CacheObjectPtr Cache::get( const CacheId& cacheId ) const
{
    if( cacheId == INVALID_CACHE_ID )
        return CacheObjectPtr();

    return _impl->get( cacheId );
}

void Cache::_unloadAll()
{
    _impl->unloadAll();
}

size_t Cache::getCount() const
{
    return _impl->getCount();
}

CacheStatistics& Cache::_getStatistics()
{
    return _impl->_statistics;
}

const CacheStatistics& Cache::getStatistics() const
{
    return _impl->_statistics;
}

}
