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

#ifndef _Promise_h_
#define _Promise_h_

#include <livre/core/pipeline/Future.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/types.h>

namespace livre
{


/**
 * Is similar to the std::promise classes in functionality and it has additional
 * information for the name and data type. It provides methods to set the data.
 */
class Promise
{
public:

    /**
     * @param dataInfo is the name and type information for the data.
     */
    Promise( const DataInfo& dataInfo );
    ~Promise();

    /**
     * @return the name of the connection
     */
    std::type_index getDataType() const;

    /**
     * @return the name of the connection
     */
    std::string getName() const;

    /**
     * Sets the port with the value.
     * @param value to be set
     * @throw std::runtime_error when the port data is not exact
     * type T or there is no such port name.
     */
    template< class T >
    void set( const T& value )
    {
        _set( std::make_shared< PortDataT< T >>( value ));
    }

    /**
     * Sets the port with the value.
     * @param value to be set
     * @throw std::runtime_error when the port data is not exact
     * type T or there is no such port name.
     */
    template< class T >
    void set( const T&& value )
    {
        _set( std::make_shared< PortDataT< T >>( value ));
    }

    /**
     * Sets the promise with empty data if it is not set already
     */
    void flush();

    /**
     * @return the future, that can be queried for data retrieval
     */
    Future getFuture() const;

    /**
     * @resets the promise. ( Future is reset and value can be set again )
     * The behavior of the function is undefined when multiple threads
     * execute query/get from future.
     */
    void reset();

private:

    void _set( const PortDataPtr& data );

    struct Impl;
    std::shared_ptr<Impl> _impl;
};

}

#endif // _Promise_h_

