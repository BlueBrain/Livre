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

#ifndef _FunctionFilter_h_
#define _FunctionFilter_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/Filter.h>

namespace livre
{

/**
 * The FunctionFilter class is used to create filters based
 * on filter function definitions. i.e. A class can implement
 * a filter by using a function.
 */
struct FunctionFilter final : public Filter
{
    /**
     * Constructs a filter based on function object.
     * @param filterFunc The filter function object.
     * @param inputPorts are the input ports information.
     * @param outputPorts are the output ports information.
     */
    FunctionFilter( const FilterFunc& filterFunc,
                    const PortInfos& inputPorts,
                    const PortInfos& outputPorts );
    ~FunctionFilter();

    /**
     * @copydoc Filter::execute( PipeFilter& filter )
     */
    void execute( PipeFilter& filter ) const final;

    /**
     * @copydoc Filter::getInputPorts()
     */
    void getInputPorts( PortInfos& inputPorts ) const final;

    /**
     * @copydoc Filter::getOutputPorts()
     */
    void getOutputPorts( PortInfos& outputPorts ) const final;

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _FunctionFilter_h_


