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

namespace livre
{

CacheLoadException::CacheLoadException( const Identifier& id,
                                        const std::string& message )
    : _id( id )
    , _message( message )
{}

const char* CacheLoadException::what() const throw()
{
    std::stringstream message;
    message << "Id: " << _id << " " << _message << std::endl;
    return message.str().c_str();
}

struct LRUCachePolicy
{
    typedef std::deque< CacheId > LRUQueue;

    LRUCachePolicy( const size_t maxMemBytes )
        : _maxMemBytes( maxMemBytes )
        , _cleanUpRatio( 1.0f )
    {}

    bool isFull( const Cache& cache ) const
    {
        const size_t usedMemBytes = cache.getStatistics().getUsedMemory();
        return usedMemBytes >= _maxMemBytes;
    }

    bool hasSpace( const Cache& cache ) const
    {
        const size_t usedMemBytes = cache.getStatistics().getUsedMemory();
        return usedMemBytes < _cleanUpRatio * _maxMemBytes;
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
            {
                _lruQueue.erase( it );
                return;
            }
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

    void clear()
    {
        _lruQueue.clear();
    }

    const size_t _maxMemBytes;
    const float _cleanUpRatio;
    LRUQueue _lruQueue;
};

struct Cache::Impl
{
    Impl( Cache& cache,
          const std::string& name,
          const size_t maxMemBytes )
        : _policy( maxMemBytes )
        , _cache( cache )
        , _statistics( name, maxMemBytes )
        , _cacheMap( 128 )
    {}

    ~Impl()
    {}

    void applyPolicy()
    {
        if( _cacheMap.empty() || !_policy.isFull( _cache ))
            return;

        // Objects are returned in delete order
        for( const CacheId& cacheId: _policy.getObjects( ))
        {
            unloadFromCache( cacheId );
            if( _policy.hasSpace( _cache ))
                return;
        }
    }

    CacheObjectPtr load( CacheObject* obj )
    {
        const CacheId& cacheId = obj->getId();
        CacheMap::const_iterator it;
        {
            ReadLock readLock( _mutex );
            it = _cacheMap.find( cacheId );
            if( it != _cacheMap.end( ))
                return it->second;
        }

        WriteLock writeLock( _mutex );
        CacheObjectPtr cacheObject( obj );
        it = _cacheMap.find( cacheId );
        if( it == _cacheMap.end( ))
            _cacheMap[ cacheId ] = cacheObject;
        _statistics.notifyMiss();
        _statistics.notifyLoaded( *obj );
        _policy.insert( cacheId );
        applyPolicy();

        return cacheObject;
    }

    bool unloadFromCache( const CacheId& cacheId )
    {
        CacheMap::iterator it = _cacheMap.find( cacheId );
        if( it == _cacheMap.end( ))
            return false;

        CacheObjectPtr& obj = it->second;
        if( obj.use_count() > 1 )
            return false;

        _statistics.notifyUnloaded( *obj );
        obj.reset();
        _policy.remove( cacheId );
        _cacheMap.erase( cacheId );
        return true;
    }

    CacheObjectPtr getFromMap( const CacheId& cacheId ) const
    {
        ReadLock readLock( _mutex );
        CacheMap::const_iterator it = _cacheMap.find( cacheId );
        if( it == _cacheMap.end( ))
            return CacheObjectPtr();

        return it->second;
    }

    bool unload( const CacheId& cacheId )
    {
        WriteLock lock( _mutex );
        return unloadFromCache( cacheId );
    }

    ConstCacheObjectPtr get( const CacheId& cacheId ) const
    {
        return getFromMap( cacheId );
    }

    size_t getCount() const
    {
        ReadLock lock( _mutex );
        return _cacheMap.size();
    }

    void purge()
    {
        WriteLock lock( _mutex );
        _statistics.clear();
        _policy.clear();
        _cacheMap.clear();
    }

    void purge( const CacheId& cacheId )
    {
        WriteLock lock( _mutex );
        _cacheMap.erase( cacheId );
    }

    mutable LRUCachePolicy _policy;
    Cache& _cache;
    mutable CacheStatistics _statistics;
    CacheMap _cacheMap;
    mutable ReadWriteMutex _mutex;
};

Cache::Cache( const std::string& name, const size_t maxMemBytes )
    : _impl( new Cache::Impl( *this, name, maxMemBytes ))
{}

Cache::~Cache()
{}

ConstCacheObjectPtr Cache::_load( CacheObject* obj )
{
    if( obj->getId() == INVALID_CACHE_ID )
        return ConstCacheObjectPtr();

    return _impl->load( obj );
}

bool Cache::unload( const CacheId& cacheId )
{
    if( cacheId == INVALID_CACHE_ID )
        return false;

    return _impl->unload( cacheId );
}

ConstCacheObjectPtr Cache::get( const CacheId& cacheId ) const
{
    if( cacheId == INVALID_CACHE_ID )
        return ConstCacheObjectPtr();

    return _impl->get( cacheId );
}

size_t Cache::getCount() const
{
    return _impl->getCount();
}

const CacheStatistics& Cache::getStatistics() const
{
    return _impl->_statistics;
}

void Cache::purge()
{
    _impl->purge();
}

void Cache::purge( const CacheId& cacheId )
{
    _impl->purge( cacheId );
}

}
