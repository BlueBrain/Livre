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

#ifndef _PipeFilter_h_
#define _PipeFilter_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/Filter.h>
#include <livre/core/pipeline/InputPort.h>
#include <livre/core/pipeline/OutputPort.h>
#include <livre/core/pipeline/Executable.h>
#include <livre/core/pipeline/FilterInput.h>
#include <livre/core/pipeline/FilterOutput.h>

namespace livre
{

/**
 * PipeFilter class instantiates the @Filter classes by constructing
 * the communication layer around the filter.
 */
class PipeFilter : public Executable
{
public:

    /**
     * Constructs a PipeFilter with a given filter
     * @param filter the filter object
     */
    PipeFilter( FilterPtr filter );

    /**
     * Constructs a PipeFilter with a given filter function
     * @param func is the filter function object
     * @param inputPorts input ports
     * @param outputPorts output ports
     */
    PipeFilter( const FilterFunc& func,
                const PortInfos& inputPorts,
                const PortInfos& outputPorts );

    ~PipeFilter();

    /**
     * Executes the filter. If a filter input port is
     * connected and no input is provided to the port
     * the execution will block.
     */
    void execute() final;

    /**
     * Queries if input port with the given name is available.
     * @param portName is the name of the port.
     * @return true if there is a port with given name
     */
    bool hasInputPort( const std::string& portName ) const;

    /**
     * Queries if output port with the given name is available.
     * @param portName is the name of the port.
     * @return true if there is a port with given name
     */
    bool hasOutputPort( const std::string& portName ) const;

    /**
     * Sets the input port with the value. If value is set, the output
     * ports are ready to retrieve the data.
     * @param portName is the name of the port.
     * @param value to be set
     * @throw std::runtime_error whem the port with the port name
     * is not available.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    void setInput( const std::string& portName, const T& value )
    {
        _getInput().getPort( portName )->setValue( value );
    }

    /**
     * Sets the input port with the value (moves the value). If value
     * is set, the output ports are ready to retrieve the data.
     * @param portName is the name of the port.
     * @param value to be set
     * @throw std::runtime_error whem the port with the port name
     * is not available.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    void setInput( const std::string& portName, const T&& value )
    {
        _getInput().getPort( portName )->setValue( value );
    }

    /**
     * Sets the output port with the value.
     * @param portName is the name of the port.
     * @param value to be set
     * @throw std::runtime_error whem the port with the port name
     * is not available.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    void setOutput( const std::string& portName, const T& value )
    {
        _getOutput().getPort( portName )->setValue( value );
    }

    /**
     * Sets the output port with the value (moves the value).
     * @param portName is the name of the port.
     * @param value to be set
     * @throw std::runtime_error whem the port with the port name
     * is not available.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    void setOutput( const std::string& portName, const T&& value )
    {
        _getOutput().getPort( portName )->setValue( value );
    }

    /**
     * Gets the value from input port with the given type T at
     * given index i. If input is connected and value is not
     * provided this function will block.
     * @param portName is the name of the port.
     * @param index of the input value
     * @return the value.
     * @throw std::runtime_error when the port data is not exact
     * type T
     * @throw std::runtime_error when index exceeds the input size.
     */
    template< class T >
    const T& getInputValue( const std::string& portName,
                            size_t index = 0 ) const
    {
        return _getInput().getPort( portName )->getValue<T>( index );
    }

    /**
     * Moves the value from input port with the given type T at
     * given index i. If input is connected and value is not
     * provided this function will block.
     * @param portName is the name of the port.
     * @param index of the input value
     * @return the value.
     * @throw std::runtime_error when the port data is not exact
     * type T
     * @throw std::runtime_error when index exceeds the input size.
     */
    template< class T >
    const T&& moveInputValue( const std::string& portName,
                            size_t index = 0 ) const
    {
        return _getInput().getPort( portName )->moveValue<T>( index );
    }

    /**
     * Gets the value from output port with the given type T.
     * @param portName is the name of the port. If output is not
     * provided this function will block.
     * @return the value.
     * @throw std::runtime_error when the port data is not exact
     * type T
     * @throw std::runtime_error when index exceeds the input size.
     */
    template< class T >
    const T& getOutputValue( const std::string& portName ) const
    {
        return _getOutput().getPort( portName )->getValue< T >();
    }

    /**
     * Moves the value from output port with the given type T.
     * @param portName is the name of the port. If output is not
     * provided this function will block.
     * @return the value.
     * @throw std::runtime_error when the port data is not exact
     * type T
     * @throw std::runtime_error when index exceeds the input size.
     */
    template< class T >
    const T&& moveOutputValue( const std::string& portName ) const
    {
        return _getOutput().getPort( portName )->moveValue< T >();
    }

    /**
     * Waits for a given port name or for all the input ports if
     * portname is empty.
     * @param portName is the name of the port. Empty for all ports.
     * @throw std::runtime_error whem the port with the port name
     * is not available.
     */
    void waitForInput( const std::string& portName = "" ) const;

    /**
     * Waits for a given port name or for all the output ports if
     * portname is empty.
     * @param portName is the name of the port. Empty for all ports.
     * @throw std::runtime_error whem the port with the port name
     * is not available.
     */
    void waitForOutput( const std::string& portName = "" ) const;

    /**
     * Waits for a any input to be ready.
     * @param readyPorts The ready ports are listed into readyPorts array.
     * @return true there are still waiting inputs.
     */
    bool waitForAnyInput( lunchbox::Strings& portNames ) const;

    /**
     * Waits for a any output to be ready.
     * @param readyPorts The ready ports are listed into readyPorts array.
     * @return true there are still waiting inputs.
     */
    bool waitForAnyOutput( lunchbox::Strings& portNames ) const;

    /**
     * Queries given port name or for all the output ports if
     * port is ready.
     * @param portName is the name of the port. Empty for all ports.
     * @return true if port/ports are ready.
     * is not available.
     * @throw std::runtime_error whem the port with the port name
     */
    bool isOutputPortReady( const std::string& portName = "") const;

    /**
     * Queries given port name or for all the input ports if
     * port is ready.
     * @param portName is the name of the port. Empty for all ports.
     * @return true if port/ports are ready.
     * is not available.
     * @throw std::runtime_error whem the port with the port name
     */
    bool isInputPortReady( const std::string& portName = "" ) const;

    /**
     * Queries for all the input ports if port is ready.
     * @return true if port/ports are ready.
     * is not available.
     */
    bool isInputReady() const final;

    /**
     * Gets the count of connections to the given input port.
     * @param portName is the name of the port.
     * @return the number of inputs.
     * @throw std::runtime_error whem the port with the port name
     * is not available.
     */
    size_t getInputSize( const std::string& portName ) const;

    /**
     * @return the unique id of the filter.
     */
    const servus::uint128_t& getId() const;

    /**
     * Connect given pipe filters with the given port name. Both filters
     * should have the same port name and their types should match.
     * @param src is the source pipe filter.
     * @param dst is the destination pipe filter.
     * @param portName connection port name.
     * @return true if connection is successful.
     */
    friend bool connectFilters( PipeFilterPtr& src,
                                PipeFilterPtr& dst,
                                const std::string& portName );

    /**
     * Connect given pipe filters. When source filter execution
     * ends, destination filter is notified.
     * @param src is the source pipe filter.
     * @param dst is the destination pipe filter.
     * @return true if connection is successful.
     */
    friend bool connectFilters( PipeFilterPtr& src,
                                PipeFilterPtr& dst );

private:

    friend class Pipeline;

    FilterPtr _getFilter();
    FilterInput& _getInput();
    FilterOutput& _getOutput();

    const FilterInput& _getInput() const;
    const FilterOutput& _getOutput() const;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

bool connectFilters( PipeFilterPtr& src,
                     PipeFilterPtr& dst,
                     const std::string& portName );

bool connectFilters( PipeFilterPtr& src,
                     PipeFilterPtr& dst );


}

#endif // _PipeFilter_h_

