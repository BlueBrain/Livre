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
 * Responsible for execution of the @see Filter objects by constructing
 * the communication layer ( output ports, input ports ) around the filter.
 */
class PipeFilter : public Executable
{
public:

    ~PipeFilter();

    /**
     * @return the unique name of the filter.
     */
    std::string getName() const;

    /**
     * Connects to given pipe filter with the given port names. Both filters
     * should have the same port data type.
     * @param srcPortName is the source pipe filter.
     * @param dst is the destination pipe filter.
     * @param dstPortName connection port name.
     * @throws std::runtime_error if connection can not be established
     */
    void connect( const std::string& srcPortName,
                  PipeFilter& dst,
                  const std::string& dstPortName );

    /**
     * @return promise for the given input port. If there is no connection to the
     * input port, a new promise is created for the port and no further connections are allowed,
     * if there is a connection getting a promise is not allowed.
     * @throws std::runtime_error if there is already a connection or if there is
     * no inputport or it is a noification port.
     */
    Promise getPromise( const std::string& portName );

    /**
     * @copydoc Executable::execute
     */
    void execute() final;

    /**
     * @copydoc Executable::getPostconditions
     */
    Futures getPostconditions() const final;

    /**
     * @copydoc Executable::getPreconditions
     */
    Futures getPreconditions() const final;

    /**
     * @copydoc Executable::reset
     */
    void reset() final;

protected:

    /**
     * Constructs a PipeFilter with a given filter
     * @param name of the pipefilter
     * @param filter the filter object.
     */
    PipeFilter( const std::string& name,
                FilterPtr&& filter );

private:

    struct Impl;
    std::shared_ptr< Impl > _impl;
};

/**
 * Creates a PipeFiter class instance with a given filter type
 */
template< class FilterT >
class PipeFilterT : public PipeFilter
{
public:

    /**
     * Constructs a PipeFilter with a given filter type FilterT
     * @param name of the pipefilter
     * @param args are the arguments for construction of FilterT
     */
    template< class... Args >
    PipeFilterT( const std::string& name,
                 Args&&... args )
        : PipeFilter( name, FilterPtr( new FilterT( args... )))
    {}
};

}

#endif // _PipeFilter_h_

