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

#ifndef _Filter_h_
#define _Filter_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/pipeline/PortInfo.h>

namespace livre
{

/**
 * Filters are similar to functions. Their inputs and
 * outputs are provided through ports. These ports provide
 * thread safe information retrieval, setting and querying.
 *
 * One input port may have many incoming connections and one output
 * port can be connected to many other input connections. The ports
 * have their unique names and these names are used to query,
 * retrieve and set data.
 *
 * Execution part of filters can be thought as functions and
 * port definitions can be thought as inputs and outputs to
 * those functions. PipeFilters build the connection and execution
 * functionality around filter instances.
 */

class Filter
{
public:

    /**
     * This function is called by the @PipeFilter while the filter
     * is being executed.
     * @param input The Future that can be read for input parameters
     * @param output The Promise that can be written to for output parameters
     */
    virtual void execute( const InFutures& input, PortPromises& output ) const = 0;

    /**
     * @param inputPorts information is filled by the Filter class.
     * These ports are constructed by the @PipeFilter.
     */
    virtual PortInfos getInputPorts() const  { return PortInfos(); }

    /**
     * @param outputPorts information is filled by the Filter class.
     * Afterwards, these ports are instantiated by the @PipeFilter.
     */
    virtual PortInfos getOutputPorts() const { return PortInfos(); }

    virtual ~Filter() {}
};

}

#endif // _Filter_h_

