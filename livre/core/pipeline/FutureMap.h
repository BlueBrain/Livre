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

#include <livre/core/types.h>
#include <livre/core/pipeline/Future.h>

namespace livre
{

/**
 * FutureMap is a wrapper class to query multiple futures with same names for data and data state.
 */
class FutureMap
{
public:

    /**
     * @param futures is the list of futures
     * future names are used for name-future association.
     */
    explicit FutureMap( const Futures& futures );
    ~FutureMap();

    /**
     * Gets the copy of value(s) with the given type T. If input
     * is connected and values are not provided this function will
     * block.
     * @param name of the future.
     * @return the values of the futures.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    ResultsT< T > get( const std::string& name ) const
    {
        ResultsT< T > results;
        for( const auto& future: getFutures( name ))
            results.push_back( future.get< T >( ));

        return results;
    }

    /**
     * Gets the copy of ready value(s) with the given type T.
     * @param name of the future.
     * @return the values of the futures.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    ResultsT< T > getReady( const std::string& name ) const
    {
        ResultsT< T > results;
        for( const auto& future: getFutures( name ))
        {
            if( !future.isReady())
                continue;

            results.push_back( future.get< T >( ));
        }
        return results;
    }

    /**
     * @param name of the future. If name is ALL_FUTURES, all futures are marked to return.
     * @return the futures associated with the name.
     */
    Futures getFutures( const std::string& portName = ALL_FUTURES ) const;

    /**
     * Queries if port is ready
     * @param name of the future. If name is ALL_FUTURES, all futures are queried
     * @return true if all port inputs are ready.
     * @throw std::runtime_error when there is no future associated with the
     * given name
     */
    bool isReady( const std::string& portName = ALL_FUTURES ) const;

    /**
     * Waits all futures associated with port name
     * @param name of the future. If name is ALL_FUTURES, all futures are waited.
     * @throw std::runtime_error when there is no future associated with the
     * given name
     */
    void wait( const std::string& portName = ALL_FUTURES ) const;

    /**
     * Waits all futures associated with port name.
     * @param name of the future. If name is ALL_FUTURES, all futures are waited.
     * @throw std::runtime_error when there is no future associated with the
     * given name
     */
    bool waitForAny( const std::string& portName = ALL_FUTURES ) const;

private:

    struct Impl;
    std::shared_ptr<Impl> _impl;
};

/**
 * UniqueFutureMap is a wrapper class to query futures with unique names for data and data state.
 */
class UniqueFutureMap
{
public:

    /**
     * @param futures the list of futures.
     * @throws std::runtime_error when futures are not unique in names
     */
    explicit UniqueFutureMap( const Futures& futures );
    ~UniqueFutureMap();

    /**
     * Gets the copy of value(s) with the given type T. If input
     * is connected and values are not provided this function will
     * block.
     * @param name of the future.
     * @return the value for the future.
     * @throw std::runtime_error when the port data is not exact
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
     * @throw std::runtime_error when the name is ALL_FUTURES
     */
    Future getFuture( const std::string& name ) const;

    /**
     * @return all the futures.
     */
    Futures getFutures() const;

    /**
     * Queries if port is ready
     * @param name of the future
     * @return true if all futures are ready.
     * @throw std::runtime_error when there is no future associated with the
     * given name
     */
    bool isReady( const std::string& name = ALL_FUTURES ) const;

    /**
     * Waits for the future associated with port name
     * @param name is the port name assoicated with futures.
     * @throw std::runtime_error when there is no future associated with the
     * given name
     */
    void wait( const std::string& name = ALL_FUTURES ) const;

    /**
     * Waits for any future to be ready.
     * @return true if there are still futures which are not ready.
     */
    bool waitForAny() const;

private:

    struct Impl;
    std::shared_ptr<Impl> _impl;
};

}

#endif // _FutureMap_h_
