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

#ifndef _PortPromises_h_
#define _PortPromises_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/Filter.h>

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
class PortPromises
{

public:

    PortPromises( const Promises& promises );
    ~PortPromises();

    /**
     * Sets the port with the value.
     * @param value to be set
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    void set( const std::string& portName, const T& value )
    {
       _set( portName, makePortDataPtr( value ));
    }

    /**
     * Sets the port with the value.
     * @param value to be set
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    void set( const std::string& portName, const T&& value )
    {
        _set( portName, makePortDataPtr( value ));
    }

    /**
     * Writes empty values to promises
     * @param portName is the port name. If ALL_PORTS is given,
     * all promises will be flushed
     */
    void flush( const std::string& portName = ALL_PORTS );

private:

    void _set( const std::string& portName, ConstPortDataPtr data );

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _PortPromises_h_
