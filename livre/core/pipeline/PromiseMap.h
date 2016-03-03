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

#ifndef _PromiseMap_h_
#define _PromiseMap_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/Promise.h>

namespace livre
{

/**
 * The PipeFilterOutput class is used by the @PipeFilter class to instantiate
 * the outgoing connections to a given filter using the connection info
 * provided by the filter.
 *
 * It provides functionality to wait on any or all the ports given by
 * name.
 *
 * It also provides thread safe functions to query the state of the port.
 */
class PromiseMap
{

public:

    explicit PromiseMap( const Promises& promises );
    ~PromiseMap();

    /**
     * @param portName is the port name.
     * @return the promise related to the port name.
     */
    PromisePtr getPromise( const std::string& portName );

    /**
     * Sets the port with the value.
     * @param portName is the port name.
     * @param value to be set
     * @throw std::runtime_error when the port data is not exact
     * type T or there is no such port name.
     */
    template< class T >
    void set( const std::string& portName, const T& value )
    {
        getPromise( portName )->set( value );
    }

    /**
     * Sets the port with the value.
     * @param portName is the port name.
     * @param value to be set
     * @throw std::runtime_error when the port data is not exact
     * type T or there is no such port name.
     */
    template< class T >
    void set( const std::string& portName, const T&& value )
    {
        getPromise( portName )->set( value );
    }

    /**
     * Writes empty values to promises which are not set already.
     * @param portName is the port name. If ALL_PORTS is given,
     * all promises will be flushed
     * @throw std::runtime_error there is no such port name.
     */
    void flush( const std::string& portName = ALL_PORTS );

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _PromiseMap_h_
