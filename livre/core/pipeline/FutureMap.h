/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#ifndef _FutureMap_h_
#define _FutureMap_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/pipeline/FuturePromise.h>

namespace livre
{

/**
 * FutureMap is a wrapper class to query the map of ( name, future )
 * futures with for data and state. In the map there can be multiple futures
 * with the same name. i.e. if there are multiple futures with the same name
 * "FutureName", the get() function will block until all the futures with
 * name "FutureName" are ready and get() will return a vector of results.
 *
 * Filters has named ports to communicate with other filters and those ports
 * are communicated through promise/future couples. The futures are named
 * with the port names and users can access those values using the port
 * names in the futures. This class provides convenient functions for querying
 * futures with port names.
 */
class FutureMap
{
public:

    /**
     * @param futures is the list of futures
     * future names are used for name-future association.
     */
    LIVRECORE_API explicit FutureMap( const Futures& futures );
    LIVRECORE_API ~FutureMap();

    /**
     * Gets a copy of value(s) with the given type T. Until all
     * futures with a given name are ready, this function will block.
     * @param name of the future.
     * @return the values of the futures.
     * @throw std::logic_error when there is no future associated with the
     * given name
     * @throw std::runtime_error when the data is not exact
     * type T
     */
    template< class T >
    std::vector< T > get( const std::string& name ) const
    {
        std::vector< T > results;
        for( const auto& future: getFutures( name ))
            results.push_back( future.get< T >( ));

        return results;
    }

    /**
     * Gets the copy of ready value(s) with the given type T.
     * @param name of the future.
     * @return the values of the futures.
     * @throw std::logic_error when there is no future associated with the
     * given name
     * @throw std::runtime_error when the data is not exact
     * type T
     */
    template< class T >
    std::vector< T > getReady( const std::string& name ) const
    {
        std::vector< T > results;
        for( const auto& future: getFutures( name ))
        {
            if( !future.isReady())
                continue;

            results.push_back( future.get< T >( ));
        }
        return results;
    }

    /**
     * @param name of the future.
     * @return the futures with the given name
     */
    LIVRECORE_API Futures getFutures( const std::string& name ) const;

    /**
     * @return the futures
     */
    LIVRECORE_API Futures getFutures() const;

    /**
     * Queries if futures are ready with a given name
     * @param name of the future.
     * @return true if all futures with the given name are ready.
     * @throw std::logic_error when there is no future associated with the
     * given name
     */
    LIVRECORE_API bool isReady( const std::string& name ) const;

    /**
     * Queries if all futures are ready
     * @return true if all futures are ready.
     * @throw std::runtime_error when there is no future associated with the
     * given name
     */
    LIVRECORE_API bool isReady() const;

    /**
     * Waits all futures associated with a given name
     * @param name of the future.
     * @throw std::logic_error when there is no future associated with the
     * given name
     */
    LIVRECORE_API void wait( const std::string& name ) const;

    /**
     * Waits all futures
     * @throw std::runtime_error when there is no future associated with the
     * given name
     */
    LIVRECORE_API void wait() const;

    /**
     * Waits all futures associated with a given name.
     * @param name of the future.
     * @throw std::logic_error when there is no future associated with the
     * given name
     */
    LIVRECORE_API void waitForAny( const std::string& name ) const;

    /**
     * Waits all futures.
     * @throw std::logic_error when there is no future associated with the
     * given name
     */
    LIVRECORE_API void waitForAny() const;

private:

    struct Impl;
    std::shared_ptr<Impl> _impl;
};

/**
 * UniqueFutureMap is similar to the  FutureMap but for each name there is
 * a unique future. So that, the value can be retrieved directly for the given
 * name.
 */
class UniqueFutureMap
{
public:

    /**
     * @param futures the list of futures.
     * @throw std::logic_error when futures are not unique in names
     */
    LIVRECORE_API explicit UniqueFutureMap( const Futures& futures );
    LIVRECORE_API ~UniqueFutureMap();

    /**
     * Gets the copy of value(s) with the given type T. If input
     * is connected and values are not provided this function will
     * block.
     * @param name of the future.
     * @return the value for the future.
     * @throw std::logic_error when there is no future associated with the
     * given name
     * @throw std::runtime_error when the data is not exact
     * type T
     */
    template< class T >
    const T& get( const std::string& name ) const
    {
         return getFuture( name ).get< T >();
    }

    /**
     * @param name of the future
     * @return the future associated with the name.
     * @throw std::logic_error when there is no future associated with the
     * given name
     */
    LIVRECORE_API Future getFuture( const std::string& name ) const;

    /**
     * @return all the futures.
     */
    LIVRECORE_API Futures getFutures() const;

    /**
     * Queries if future is ready for a given name
     * @param name of the future
     * @return true if all futures with the given name are ready.
     * @throw std::logic_error when there is no future associated with the
     * given name
     */
    LIVRECORE_API bool isReady( const std::string& name ) const;

    /**
     * Queries if future is ready for a given name
     * @return true if all futures are ready.
     * @throw std::logic_error when there is no future associated with the
     * given name
     */
    LIVRECORE_API bool isReady() const;

    /**
     * Waits for the future associated with a given name
     * @param name is the name assoicated with futures.
     * @throw std::logic_error when there is no future associated with the
     * given name
     */
    LIVRECORE_API void wait( const std::string& name ) const;

    /**
     * Waits for all the futures
     * @throw std::logic_error when there is no future associated with the
     * given name
     */
    LIVRECORE_API void wait() const;

    /**
     * Waits for any future to be ready.
     */
    LIVRECORE_API void waitForAny() const;

private:

    struct Impl;
    std::shared_ptr<Impl> _impl;
};

}

#endif // _FutureMap_h_
