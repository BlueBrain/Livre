/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#ifndef _OutputPort_h_
#define _OutputPort_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/PortData.h>

namespace livre
{

class OutputPort
{

public:

    OutputPort( const PortInfo& portInfo );
    ~OutputPort();

    /**
     * @return name of the port
     */
    const std::string& getName() const;

    /**
     * Gets the value with the given type T. If output is not set
     * this function will block.
     * @return the value.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    const T& getValue() const
    {
        typedef typename std::remove_const<T>::type UnconstT;
        const auto& dataPtr =
                boost::dynamic_pointer_cast< const PortDataT< UnconstT >>( _getValue( ));

        if( !dataPtr )
            LBTHROW( std::runtime_error( "Invalid data type" ));

        return dataPtr->data;
    }

    /**
     * Moves the value with the given type T. If output is not set
     * this function will block.
     * @return the value.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    const T&& moveValue() const
    {
        typedef typename std::remove_const<T>::type UnconstT;
        const auto& dataPtr =
                boost::dynamic_pointer_cast< const PortDataT< UnconstT >>( _getValue( ));

        if( !dataPtr )
            LBTHROW( std::runtime_error( "Invalid data type" ));

        return std::move( dataPtr->data );
    }

    /**
     * Sets the port with the value.
     * @param value to be set
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    void setValue( const T& value )
    {
        const auto& message = makePortDataPtr( value );
        _setValue( message );
    }

    /**
     * Sets the port with the value.
     * @param value to be set
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    void setValue( const T&& value )
    {
        const auto& message = makePortDataPtr( value );
        _setValue( message );
    }

    /**
     * Waits until the data is ready.
     */
    void wait() const;

    /**
     * Sets the port with default value.
     */
    void flush();

    /**
     * @return true if port is ready for data retrieval.
     */
    bool isReady() const;

    /**
     * Waits for any of the given output ports
     * @param ports is the list of output port
     * @return true if there are still waiting ports.
     */
    friend bool waitForAny( const ConstOutputPortPtrs& ports );

    /**
     * Waits for all of the given output ports
     * @param ports is the list of output port
     */
    friend void waitForAll( const ConstOutputPortPtrs& ports );

    /**
     * Connects input and output ports
     * @param inputPort the input port
     * @param outputPort the output port
     */
    friend void connectPorts( InputPortPtr inputPort,
                              OutputPortPtr outputPort );

private:

    ConstPortDataPtr _getValue() const;
    void _setValue( ConstPortDataPtr msg );
    ConstPortDataPtr _getDefaultValue() const;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

bool waitForAny( const ConstOutputPortPtrs& ports );
void waitForAll( const ConstOutputPortPtrs& ports );

}

#endif // _OutputPort_h_

