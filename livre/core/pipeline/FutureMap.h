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
 * The Futures classes are a wrappers to access the group of future
 * and execute thread-safe operations on them. With a given portName (name)
 * many futures can be associated.
 */

/**
 * FutureMap class provides basic functionality to query state of multiple futures.
 */
class FutureMap
{
public:

    /**
     * @param futures the list of futures.
     */
    explicit FutureMap( const Futures& futures );
    ~FutureMap();

    /**
     * @return the futures associated with port name.
     */
    Futures getFutures() const;

    /**
     * Queries if all ports are ready
     * @return true if all port inputs are ready.
     */
    bool isReady() const;

    /**
     * Waits all futures
     */
    void wait() const;

    /**
     * Waits for any futures
     * @return true if any future is waited.
     */
    bool waitForAny() const;

private:

    struct Impl;
    std::shared_ptr<Impl> _impl;
};

/**
 * PortFutures for managing ports with unique names.
 */
class OutFutureMap
{
public:

    /**
     * @param futures the list of futures.
     * @throws std::runtime_error when futures are not unique in names
     */
    explicit OutFutureMap( const Futures& futures );
    ~OutFutureMap();

    /**
     * Gets the copy of value(s) with the given type T. If input
     * is connected and values are not provided this function will
     * block.
     * @param portName is the port name assoicated with futures.
     * @return the values port, value map.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    const T& get( const std::string& portName ) const
    {
         return getFuture( portName ).get< T >();
    }

    /**
     * Moves the value(s) with the given type T. If input
     * is connected and values are not provided this function will
     * block.
     * @param portName is the port name assoicated with futures.
     * @return the values vector.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    const T&& move( const std::string& portName ) const
    {
        return getFuture( portName ).move< T >();
    }

    /**
     * @param portName is the port name assoicated with the future. If port name
     * is ALL_PORTS, all futures are marked to return.
     * @return the futures associated with port name.
     */
    Future getFuture( const std::string& portName ) const;

    /**
     * @param portName is the port name assoicated with futures. If port name
     * is ALL_PORTS, all futures are marked to return.
     * @return the future associated with port name.
     */
    Futures getFutures() const;

    /**
     * Queries if port is ready
     * @param portName is the port name assoicated with the future.
     * @return true if all port inputs are ready.
     * @throw std::runtime_error when there is no future associated with
     * given port name
     */
    bool isReady( const std::string& portName = ALL_PORTS ) const;

    /**
     * Waits for the future associated with port name
     * @param portName is the port name assoicated with futures.
     * @throw std::runtime_error when there is no future associated with
     * given port name
     */
    void wait( const std::string& portName = ALL_PORTS ) const;

    /**
     * Waits for any futures.
     * @return true if any future is waited.
     */
    bool waitForAny() const;

private:

    struct Impl;
    std::shared_ptr<Impl> _impl;
};

/**
 * PortFutures for managing ports with non unique named ports.
 */
class InFutureMap
{
public:

    /**
     * @param inputPorts is the list of input ports
     * future names are used for name-future association.
     */
    explicit InFutureMap( const InputPorts& inputPorts );
    ~InFutureMap();

    /**
     * Gets the copy of value(s) with the given type T. If input
     * is connected and values are not provided this function will
     * block.
     * @param portName is the port name assoicated with futures.
     * @return the values port, value map.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    ResultsT< T > get( const std::string& portName ) const
    {
        ResultsT< T > results;
        for( const auto& future: getFutures( portName ))
            results.push_back( future.get< T >( ));

        return results;
    }

    /**
     * Moves the value(s) with the given type T. If input
     * is connected and values are not provided this function will
     * block.
     * @param portName is the port name assoicated with futures.
     * @return the values vector.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    ResultsT< T > move( const std::string& portName ) const
    {
        ResultsT< T > results;
        for( const auto& future: getFutures( portName ))
            results.push_back( future.move< T >( ));

        return results;
    }

    /**
     * Gets the copy of ready value(s) with the given type T.
     * @param portName is the port name assoicated with futures.
     * @return the values vector.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    ResultsT< T > getReady( const std::string& portName ) const
    {
        ResultsT< T > results;
        for( const auto& future: getFutures( portName ))
        {
            if( !future.isReady())
                continue;

            results.push_back( future.get< T >( ));
        }
        return results;
    }

    /**
     * Moves the ready value(s) with the given type T
     * @param portName is the port name assoicated with futures.
     * @return the values vector.
     * @throw std::runtime_error when the port data is not exact
     * type T or there is no future assigned to given port name
     */
    template< class T >
    ResultsT< T > moveReady( const std::string& portName ) const
    {
        ResultsT< T > results;
        for( const auto& future: getFutures( portName ))
        {
            if( !future.isReady())
                continue;

            results.push_back( future.move< T >( ));
        }
        return results;
    }

    /**
     * @param portName is the port name assoicated with futures. If port name
     * is ALL_PORTS, all futures are marked to return.
     * @return the futures associated with port name.
     */
    Futures getFutures( const std::string& portName ) const;

    /**
     * Queries if port is ready
     * @param portName is the port name assoicated with futures.
     * @return true if all port inputs are ready.
     * @throw std::runtime_error when there is no future associated with
     * given port name
     */
    bool isReady( const std::string& portName ) const;

    /**
     * Waits all futures associated with port name
     * @param portName is the port name assoicated with futures.
     * @throw std::runtime_error when there is no future associated with
     * given port name
     */
    void wait( const std::string& portName ) const;

    /**
     * Waits all futures associated with port name.
     * @param portName is the port name assoicated with futures.
     * @throw std::runtime_error when there is no future associated with
     * given port name
     */
    bool waitForAny( const std::string& portName ) const;

private:

    struct Impl;
    std::shared_ptr<Impl> _impl;
};

}

#endif // _FutureMap_h_
