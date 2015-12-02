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

#ifndef _FiterInput_h_
#define _FiterInput_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/InputPort.h>
#include <livre/core/pipeline/PortData.h>

namespace livre
{

/**
 * The FilterInput class is used by the @PipeFilter class to instantiate
 * the incoming connections to a given filter using the connection info
 * provided by the filter.
 *
 * It provides functionality to wait on any or all the ports given by
 * name.
 *
 * It also provides thread safe functions to query the state of the port.
 */
class FilterInput
{
public:

    FilterInput();
    ~FilterInput();

    /**
     * Gets the port according to given name.
     * @param portName is the name of the port.
     * @return the input port
     * @throw std::runtime_error if the port with the port name
     * is not available.
     */
    ConstInputPortPtr getPort( const std::string& portName ) const;

    /**
     * Queries if port with the given name is available.
     * @param portName is the name of the port.
     * @return true if there is a port with given name
     */
    bool hasPort( const std::string& portName ) const;

    /**
     * Waits for a given port name or for all the input ports if
     * portname is empty.
     * @param portName is the name of the port. Empty for all ports.
     * @throw std::runtime_error if the port with the port name
     * is not available.
     */
    void wait( const std::string& portName = "" ) const;

    /**
     * Waits for a any ready input to a given port ( The port
     * can have many inputs )
     * @param portName is the name of the port.
     * @return true there are still waiting inputs.
     * @throw std::runtime_error if the port with the port name
     * is not available.
     */
    bool waitForAny( const std::string& portName ) const;

    /**
     * Waits for a any input to be ready.
     * @param readyPorts The ready ports are listed into readyPorts array.
     * @return true there are still waiting inputs.
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
     * @return the list of port names.
     */
    lunchbox::Strings getPortNames() const;

    /**
     * @return true if there are no ports.
     */
    bool isEmpty() const;

    /**
     * Gets the count of connections to the given input port.
     * @param portName is the name of the port.
     * @return the number of inputs.
     * @throw std::runtime_error if the port with the port name
     * is not available.
     */
    size_t getInputSize( const std::string& portName ) const;

    /**
     * @param portName is the name of the port.
     * @return the input port
     * @throw std::runtime_error if the port with the port name
     * is not available.
     */
    InputPortPtr getPort( const std::string& portName );

    /**
     * Adds a new input port according to the given port info.
     * @param portInfo The port information
     * @return the generated code.
     * @throw std::runtime_error if the port with the port name
     * is already registered.
     */
    InputPortPtr addPort( const PortInfo& portInfo );

    /**
     * Clears the ports.
     */
    void clear();

    /**
     * Connect the given pipe filters with the given port name. Both filters
     * should have the port name.
     * @param src is the source pipe filter.
     * @param dst is the destination pipe filter.
     * @param portName connection port name.
     * @return true if connection is successful.
     */
    friend bool connectFilters( PipeFilterPtr src,
                                PipeFilterPtr dst,
                                const std::string& portName );

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _FiterInput_h_