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

#ifndef _FiterOutput_h_
#define _FiterOutput_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/Filter.h>
#include <livre/core/pipeline/OutputPort.h>

namespace livre
{

/**
 * The FilterOutput class is used by the @PipeFilter class to instantiate
 * the outgoing connections to a given filter using the connection info
 * provided by the filter.
 *
 * It provides functionality to wait on any or all the ports given by
 * name.
 *
 * It also provides thread safe functions to query the state of the port.
 */
class FilterOutput
{

public:

    FilterOutput();
    ~FilterOutput();

    /**
     * Gets the port according to given name.
     * @param portName is the name of the port.
     * @return the output port
     * @throw std::runtime_error if the port with the port name
     * is not available.
     */
    ConstOutputPortPtr getPort( const std::string& portName ) const;

    /**
     * Queries if a port with the given name is available.
     * @param portName is the name of the port.
     * @return true if there is a port with given name
     */
    bool hasPort( const std::string& portName ) const;

    /**
     * Gets the value with the given type T.
     * @param portName is the name of the port.
     * @return the value reference.
     * @throw std::runtime_error if the port with the port name
     * is not available.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    const T& getValue( const std::string& portName ) const
    {
        if(!hasPort( portName ))
            LBTHROW( std::runtime_error( std::string( "No output port with name: ")
                                                      + portName ));

        return getPort( portName )->getValue< T >();
    }

    /**
     * Moves the value with the given type T
     * @param portName is the name of the port.
     * @return the value.
     * @throw std::runtime_error if the port with the port name
     * is not available.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    const T&& moveValue( const std::string& portName ) const
    {
        if(!hasPort( portName ))
            LBTHROW( std::runtime_error( std::string( "No output port with name: ")
                                                      + portName ));

        return getPort( portName )->moveValue< T >();
    }

    /**
     * Sets the value with the given type T
     * @param portName is the name of the port.
     * @return the value reference.
     * @throw std::runtime_error if the port with the port name
     * is not available.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    void setValue( const std::string& portName,
                   const T& value )
    {
        if(!hasPort( portName ))
            LBTHROW( std::runtime_error( std::string( "No output port with name: ")
                                                      + portName ));

        return getPort( portName )->setValue( value );
    }

    /**
     * @return the list of port names.
     */
    lunchbox::Strings getPortNames() const;

    /**
     * Waits for a given port name or for all the output ports if
     * portname is empty.
     * @param portName is the name of the port. Empty for all ports.
     * @throw std::runtime_error if the port with the port name
     * is not available.
     */
    void wait( const std::string& portName = "" ) const;

    /**
     * Waits for any input to be ready.
     * @param readyPorts The ready ports are listed into readyPorts array.
     * @return true there are still waiting outputs.
     */
    bool waitForAny( lunchbox::Strings& readyPorts ) const;

    /**
     * Queries given port name or for all the input ports if
     * port is ready.
     * @param portName is the name of the port. Empty for all ports.
     * @return true if port/ports are ready.
     * is not available.
     * @throw std::runtime_error if the port with the port name
     */
    bool isReady( const std::string& portName = "" ) const;

    /**
     * Connect given pipe filters with the given port name. Both filters
     * should have the same port name and their types should match.
     * @param src is the source pipe filter.
     * @param dst is the destination pipe filter.
     * @param portName connection port name.
     * @return true if connection is successful.
     */
    friend bool connectFilters( PipeFilterPtr src,
                                PipeFilterPtr dst,
                                const std::string& portName );

    /**
     * Adds a new input port according to the given port info.
     * @param portInfo The port information
     * @return the generated code.
     * @throw std::runtime_error if the port with the port name
     * is already registered.
     */
    OutputPortPtr addPort( const PortInfo& portInfo );

    /**
     * Gets the port according to given name.
     * @param portName is the name of the port.
     * @return the input port
     * @throw std::runtime_error if the port with the port name
     * is not available.
     */
    OutputPortPtr getPort( const std::string& portName );

    /**
     * @return the output ports
     */
    ConstOutputPortPtrs getPorts() const;

    /**
     * If a value to the port, is not provided by filter, this function
     * sets the port output with default value that is defined by the port
     * information.
     */
    void flush();

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _FiterOutput_h_
