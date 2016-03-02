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

#include <livre/core/pipeline/FunctionFilter.h>
#include <livre/core/pipeline/PortInfo.h>

namespace livre
{

struct FunctionFilter::Impl
{
    Impl( const FilterFunc& filterFunc,
          const PortInfos& inputPorts,
          const PortInfos& outputPorts )
        : _filterFunc( filterFunc )
        , _inputPorts( inputPorts )
        , _outputPorts( outputPorts )
    {}

    void execute( const InFutures& input, PortPromises& output ) const
    {
        _filterFunc( filter );
    }

    PortInfos getInputPorts() const
    {
        return _inputPorts;
    }

    PortInfos getInputPorts() const
    {
        return _outputPorts;
    }

private:


    FilterFunc _filterFunc;
    const PortInfos _inputPorts;
    const PortInfos _outputPorts;
};

FunctionFilter::FunctionFilter( const FilterFunc& filterFunc,
                                const PortInfos& inputPorts,
                                const PortInfos& outputPorts )
    : _impl( new FunctionFilter::Impl( filterFunc,
                                       inputPorts,
                                       outputPorts ))
{}

FunctionFilter::~FunctionFilter()
{}

void FunctionFilter::execute( const InFutures& input, PortPromises& output ) const
{
    _impl->execute( input, output );
}

PortInfos FunctionFilter::getInputPorts() const
{
    _impl->getInputPorts();
}

PortInfos FunctionFilter::getOutputPorts( ) const
{
    _impl->getOutputPorts();
}

}



