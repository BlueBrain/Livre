/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _Cache_h_
#define _Cache_h_

#include <livre/core/api.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * Thrown by CacheObjects when loading fails
 */
class CacheLoadException : public std::exception
{
public:
    CacheLoadException( const Identifier& id, const std::string& message );
    ~CacheLoadException() throw() {}
    char const* what() const throw();

private:
    Identifier _id;
    std::string _message;
};

/**
 * The Cache class manages the \see CacheObjects according to LRU Policy, methods
 * are thread safe inserting/querying nodes. The type safety check is done in runtime.
 */
class Cache
{
public:

    LIVRECORE_API virtual ~Cache();

    /**
     * Gets the cached object from the cache.
     * @param cacheId The object cache id to be queried.
     * @return The cache object from cache, if object is not in the list an empty cache
     * object is returned.
     */
    LIVRECORE_API ConstCacheObjectPtr get( const CacheId& cacheId ) const;

    /**
     * Gets the cached object from the cache with a given type and d
     * @param cacheId The object cache id to be queried.
     * @return The cache object from cache, if object is not in the list an empty cache
     * object is returned.
     * @throw std::runtime_error if cached object is not CacheObjectT type
     */
    template< class CacheObjectT >
    LIVRECORE_API std::shared_ptr< const CacheObjectT > get( const CacheId& cacheId ) const
    {
        if( _getCacheObjectType() != getType< CacheObjectT >( ))
            LBTHROW( std::runtime_error( "The cache type casting failed for cached object" ));

        ConstCacheObjectPtr obj = get( cacheId );
        if( !obj )
            return std::shared_ptr< const CacheObjectT >();

        return std::static_pointer_cast< const CacheObjectT >( obj );
    }

    /**
     * Unloads the object from the memory, if there are not any references. The
     * objects are removed from cache
     * @param cacheId The object cache id to be unloaded.
     * @return false if object is not unloaded or cacheId is invalid
     */
    LIVRECORE_API bool unload( const CacheId& cacheId );

    /**
     * @return The number of cache objects managed.
     */
    LIVRECORE_API size_t getCount() const;

    /**
     * Loads the object to cache. If object is not in the cache it is created.
     * @param cacheId the id of the cache object to be loaded
     * @param args parameters of the cache object constructor. If there is already
     * a cache object with the same cache id, the args are not considered.
     * @return the loaded or previously loaded cache object. Return empty pointer
     * if cache id is invalid or object cannot be loaded.
     */
    template< class CacheObjectT, class... Args >
    LIVRECORE_API std::shared_ptr< const CacheObjectT > load( const CacheId& cacheId,
                                                              Args&&... args )
    {
        if( _getCacheObjectType() != getType< CacheObjectT >( ))
            LBTHROW( std::runtime_error( "The cache does not support the type" ));

        std::shared_ptr< const CacheObjectT > obj = get< CacheObjectT >( cacheId );
        if( obj )
            return obj;

        try
        {
            ConstCacheObjectPtr cacheObject
                    = _load( ConstCacheObjectPtr( new CacheObjectT( cacheId, args... )));

            std::shared_ptr< const CacheObjectT > typedObj =
                    std::dynamic_pointer_cast< const CacheObjectT >( cacheObject );

            if( !typedObj )
                LBTHROW( std::runtime_error( "The cache type casting failed for cached object" ));

            return typedObj;
        }
        catch( const CacheLoadException& )
        {}

        return obj;
    }

    /**
     * @return Statistics.
     */
    LIVRECORE_API const CacheStatistics& getStatistics() const;

    /**
     * Purges the cache by removing cached objects. The purged objects are not unloaded
     * and they will be in memory until no reference is left.
     */
    LIVRECORE_API void purge();

    /**
     * Purges a cached object from the cache. The purged object is not unloaded
     * and they will be in memory until no reference is left.
     * @param cacheId The object cache id to be purged.
     */
    LIVRECORE_API void purge( const CacheId& cacheId );

protected:

    /**
     * @param name is the name of the cache.
     * @param maxMemBytes maximum memory.
     * @param cacheObjectType type info for the cached object.
     */
    LIVRECORE_API Cache( const std::string& name,
                         size_t maxMemBytes,
                         const std::type_index& cacheObjectType );

private:

    ConstCacheObjectPtr _load( ConstCacheObjectPtr cacheObject );
    const std::type_index& _getCacheObjectType() const;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

/** Helper class for enabling cache to be constructed for CacheObjects */
template< class CacheObjectT >
class CacheT : public Cache
{
public:
    template< class Q = CacheObjectT >
    LIVRECORE_API CacheT( const std::string& name, size_t maxMemBytes,
            typename std::enable_if< std::is_base_of< CacheObject, Q >::value, Q >::type* = 0 )
        : Cache( name, maxMemBytes, getType< CacheObjectT >( ))
    {}
};

}

#endif // _Cache_h_
