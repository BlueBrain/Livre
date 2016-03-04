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

#ifndef _PipeFilter_h_
#define _PipeFilter_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/Executable.h>

namespace livre
{

/**
 * PipeFilter class instantiates the @Filter classes by constructing
 * the communication layer around the filter. While adding the ports,
 * it add its name as prefix to port name. i.e. if the name of the
 * pipefilter is "Rescale" and output port name for the filter is "Volume"
 * the port will be named as
 */
class PipeFilter : public Executable
{
public:

    /**
     * Constructs a PipeFilter with a given filter
     * @param name of the pipefilter
     * @param filter the filter object
     */
    PipeFilter( const std::string& name,
                FilterPtr filter );

    /**
     * Constructs a PipeFilter with a given filter function
     * @param name of the pipefilter
     * @param func is the filter function object
     * @param inputPorts input ports
     * @param outputPorts output ports
     */
    PipeFilter( const std::string& name,
                const FilterFunc& func,
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
     * @return the unique name of the filter.
     */
    const std::string& getName() const;

    /**
     * Connect to given pipe filter with the given port names. Both filters
     * should have the same port data type.
     * @param srcPortName is the source pipe filter.
     * @param dst is the destination pipe filter.
     * @param dstPortName connection port name.
     * @throws std::runtime_error if connection can not be established
     */
    void connect( const std::string& srcPortName,
                  PipeFilterPtr dst,
                  const std::string& dstPortName );

    /**
     * Connect to given pipe filter for notification when source filter execution
     * is complete.
     * @param dst is the destination pipe filter.
     * @return true if connection is successful.
     * @throws std::runtime_error if connection can not be established or if the
     * port already is set from outside.
     */
    void connect( PipeFilterPtr dst );

    /**
     * @copydoc Executable::getOutFutures()
     */
    Futures getPostconditions() const final;

    /**
     * @copydoc Executable::getInputFutures()
     * @note PipeFilter guarantees that only connected input futures are returned.
     */
    Futures getPreconditions() const final;

    /**
     * @return return promise for the given input port. If there is no connection to the
     * input port, a new promise is created for the port and no further connections are allowed,
     * if there is a connection getting a promise is not allowed.
     * @throws std::runtime_error if there is already a connection or if there is
     * no inputport or it is a noification port.
     */
    PromisePtr getPromise( const std::string& portName );

    /**
     * @return the unique id of the filter.
     */
    const servus::uint128_t& getId() const;

    /**
     * Resets the filter. At this point pipe filter execution should be complete.
     */
    void reset();

private:

    friend class Pipeline;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _PipeFilter_h_

