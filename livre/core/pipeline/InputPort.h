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

#ifndef _InputPort_h_
#define _InputPort_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/pipeline/PortInfo.h>
#include <livre/core/pipeline/OutputPort.h>

namespace livre
{

/**
 * InputPort class implements the input port to a @PipeFilter. It
 * provides thread safe query and retrieval.
 */
class InputPort
{

public:

    /**
     * InputPort constructor based on port information
     * @param portInfo is the port information
     */
    InputPort( const PortInfo& portInfo );
    ~InputPort();

    /**
     * @return name of the port
     */
    const std::string& getName() const;

    /**
     * @return the number of the inputs to the port
     */
    size_t getSize() const;

    /**
     * @return true if port is ready for data retrieval. If multiple
     * outputs are connected waits for all the outputs to finish.
     */
    bool isReady() const;

    /**
     * Waits until the data is ready. If multiple
     * outputs are connected waits for all the outputs.
     */
    void wait() const;

    /**
     * Waits until any of the inputs is ready.
     */
    bool waitForAny() const;

    /**
     * Gets the copy of value(s) with the given type T. If input
     * is connected and values are not provided this function will
     * block.
     * @return the values vector.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    std::vector< T > getValues() const
    {
        std::vector< T > results;
        BOOST_FOREACH( const ConstOutputPortPtr& port, getConnectedPorts( ))
        {
            const T& result = port->getValue<T>();
            results.push_back( result );
        }

        return results;
    }

    /**
     * Moves the value(s) with the given type T. If input
     * is connected and values are not provided this function will
     * block.
     * @return the values vector.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    std::vector< T > moveValues() const
    {
        std::vector< T > results;
        BOOST_FOREACH( const ConstOutputPortPtr& port, getConnectedPorts( ))
            results.push_back( port->moveValue<T>() );

        return results;
    }

    /**
     * Gets the value with the given type T at given index i.
     * If input is connected and value is not provided this function
     * will block.
     * @param index of the input value
     * @return the value.
     * @throw std::runtime_error when the port data is not exact
     * type T
     * @throw std::runtime_error when index exceeds the input size.
     */
    template< class T >
    const T& getValue( const size_t index ) const
    {
        if( getSize() == 0 ) /* No connection */
        {
            typedef typename std::remove_const<T>::type UnconstT;
            const auto& dataPtr =
                boost::dynamic_pointer_cast< const PortDataT< UnconstT >>( _getDefaultValue( ));
            if( !dataPtr )
                LBTHROW( std::runtime_error( "Invalid data type" ));

            return dataPtr->data;
        }

        if( index >= getSize( ))
            LBTHROW( std::runtime_error( "Index exceeds message count" ));

        const T& result = getConnectedPorts()[ index ]->getValue<T>();
        return result;
    }

    /**
     * Moves the value with the given type T at given index i.
     * If input is connected and value is not provided this function
     * will block.
     * @param index of the input value
     * @return the value.
     * @throw std::runtime_error when the port data is not exact
     * type T
     * @throw std::runtime_error when index exceeds the input size.
     */
    template< class T >
    const T&& moveValue( const size_t index ) const
    {
        if( getSize() == 0 ) /* No connection */
        {
            typedef typename std::remove_const<T>::type UnconstT;
            const auto& dataPtr =
                boost::dynamic_pointer_cast< const PortDataT< UnconstT >>( _getDefaultValue( ));
            if( !dataPtr )
                LBTHROW( std::runtime_error( "Invalid data type" ));

            return std::move( dataPtr->data );
        }

        if( index >= getSize( ))
            LBTHROW( std::runtime_error( "Index exceeds message count" ));

        const T& result = getConnectedPorts()[ index ]->getValue<T>();
        return std::move( result );
    }

    /**
     * Gets the copy of ready value(s) with the given type T.
     * @return the values vector.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    std::vector< T > getReadyValues() const
    {
        std::vector< T > results;
        BOOST_FOREACH( const ConstOutputPortPtr& port, getConnectedPorts( ))
        {
            if( !port->isReady())
                continue;

            const T& result = port->getValue<T>();
            results.push_back( result );
        }
        return results;
    }

    /**
     * Moves the ready value(s) with the given type T
     * @return the values vector.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    std::vector< T > moveReadyValues() const
    {
        std::vector< T > results;
        BOOST_FOREACH( const ConstOutputPortPtr& port, getConnectedPorts( ))
        {
            if( !port->isReady())
                continue;

            results.push_back( port->moveValue<T>() );
        }
        return results;
    }

    /**
     * Sets the port with the value. If value is set, the output ports are
     * ready to retrieve the data.
     * @param value to be set
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    void setValue( const T& value )
    {
        OutputPortPtr port( new OutputPort( makePortInfo( getName(), value)));
        port->setValue( value );
        _connect( port );
    }

    /**
     * Sets the port with the value. If value is set, the output ports are
     * ready to retrieve the data.
     * @param value to be set
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    void setValue( const T&& value )
    {
        OutputPortPtr port( new OutputPort( makePortInfo( getName(), value)));
        port->setValue( value );
        _connect( port );
    }

    /**
     * @return the list of connectedPorts
     */
    ConstOutputPortPtrs getConnectedPorts() const;

    /**
     * Connects input and output ports
     * @param inputPort the input port
     * @param outputPort the output port
     */
    friend void connectPorts( InputPortPtr inputPort,
                              OutputPortPtr outputPort );
private:

    OutputPortPtrs& _getConnectedPorts();
    ConstPortDataPtr _getDefaultValue() const;
    void _connect( OutputPortPtr outputPort );

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

void connectPorts( InputPortPtr inputPort,
              OutputPortPtr outputPort );

}

#endif // _InputPort_h_

