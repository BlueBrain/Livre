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

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/pipeline/Executable.h>

namespace livre
{

/**
 * Responsible for execution of the Filter objects by constructing
 * the communication layer ( output ports, input ports ) around the filter.
 * Accesing the copies of the object from other threads for non-const functions
 * is not thread safe.
 */
class PipeFilter : public Executable
{
public:

    LIVRECORE_API ~PipeFilter();

    /**
     * @return the unique name of the filter.
     */
    LIVRECORE_API std::string getName() const;

    /**
     * Connects to given pipe filter with the given port names. Both filters
     * should have the same port data type.
     * @param srcPortName is the source pipe filter.
     * @param dst is the destination pipe filter.
     * @param dstPortName connection port name.
     * @throw std::runtime_error if connection can not be established
     */
    LIVRECORE_API void connect( const std::string& srcPortName,
                  PipeFilter& dst,
                  const std::string& dstPortName );

    /**
     * @return promise for the given input port. If there is no connection to the
     * input port, a new promise is created for the port and no further connections are allowed,
     * if there is a connection getting a promise is not allowed.
     * @throw std::logic_error if there is already a connection if there is
     * no input port or it is a notification port.
     */
    LIVRECORE_API Promise getPromise( const std::string& portName );

    /**
     * @copydoc Executable::execute
     */
    LIVRECORE_API void execute() final;

    /**
     * @copydoc Executable::getPostconditions
     */
    LIVRECORE_API Futures getPostconditions() const final;

    /**
     * @copydoc Executable::getPreconditions
     */
    LIVRECORE_API Futures getPreconditions() const final;

    /**
     * @copydoc Executable::reset
     */
    LIVRECORE_API void reset() final;

protected:

    /**
     * Constructs a PipeFilter with a given filter
     * @param name of the pipefilter
     * @param filter the filter object.
     */
    PipeFilter( const std::string& name,
                FilterPtr&& filter );

private:

    ExecutablePtr clone() const;

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

