
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

#include <livre/core/pipeline/InputPort.h>
#include <livre/core/pipeline/OutputPort.h>
#include <livre/core/pipeline/FilterInput.h>
#include <livre/core/pipeline/FilterOutput.h>
#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/pipeline/PortInfo.h>
#include <livre/core/pipeline/FunctionFilter.h>

namespace livre
{

bool connectFilters( PipeFilterPtr src,
                     PipeFilterPtr dst,
                     const std::string& portName )
{
    InputPortPtr input =
            dst->_getInput().getPort( portName );

    OutputPortPtr output =
            src->_getOutput().getPort( portName );

    livre::connectPorts( input, output );
    return true;
}

bool connectFilters( PipeFilterPtr src,
                     PipeFilterPtr dst )
{
    InputPortPtr input =
            dst->_getInput().getPort( dst->getId().getString( ));

    OutputPortPtr output =
            src->_getOutput().getPort( src->getId().getString( ));

    livre::connectPorts( input, output );
    return true;
}


struct PipeFilter::Impl
{
    Impl( PipeFilter* pipeFilter,
          FilterPtr filter )
        : _pipeFilter( pipeFilter )
        , _id( servus::make_UUID( ))
        , _filter( filter )
    {
        PortInfos inputPorts, outputPorts;
        filter->getInputPorts( inputPorts );
        filter->getOutputPorts( outputPorts );
        addPorts( inputPorts, outputPorts );

    }

    Impl( PipeFilter* pipeFilter,
          const FilterFunc& filterFunc,
          const PortInfos& inputPorts,
          const PortInfos& outputPorts )
        : _pipeFilter( pipeFilter )
        , _id( servus::make_UUID( ))
        , _filter( new FunctionFilter( filterFunc,
                                       inputPorts,
                                       outputPorts ))
    {
        addPorts( inputPorts, outputPorts );
    }

    void addPorts( const PortInfos& inputPorts, const PortInfos& outputPorts )
    {
        BOOST_FOREACH( const PortInfo& input, inputPorts )
            _filterInput.addPort( input );

        _filterInput.addPort( PortInfo( _id.getString(), ConstPortDataPtr( )));

        BOOST_FOREACH( const PortInfo& output, outputPorts )
            _filterOutput.addPort( output );

        _filterOutput.addPort( PortInfo( _id.getString(), ConstPortDataPtr( )));
    }

    bool hasOutputPort( const std::string& portName ) const
    {
        return _filterOutput.hasPort( portName );
    }

    bool hasInputPort( const std::string& portName ) const
    {
        return _filterInput.hasPort( portName );
    }

    void execute()
    {
        _filter->execute( *_pipeFilter );
        _filterOutput.flush();
    }

    void waitForInput( const std::string& portName ) const
    {
        _filterInput.wait( portName );
    }

    void waitForOutput( const std::string& portName ) const
    {
        _filterOutput.wait( portName );
    }

    bool waitForAnyOutput( lunchbox::Strings& portNames ) const
    {
        return _filterOutput.waitForAny( portNames );
    }

    bool waitForAnyInput( lunchbox::Strings& portNames ) const
    {
        return _filterInput.waitForAny( portNames );
    }

    bool isInputPortReady( const std::string& portName ) const
    {
        return _filterInput.isReady( portName );
    }

    bool isOutputPortReady( const std::string& portName ) const
    {
        return _filterOutput.isReady( portName );
    }

    size_t getInputSize( const std::string& portName ) const
    {
        return _filterInput.getInputSize( portName );
    }

    PipeFilter* _pipeFilter;
    servus::uint128_t _id;
    FilterInput _filterInput;
    FilterOutput _filterOutput;
    FilterPtr _filter;
};

PipeFilter::PipeFilter( FilterPtr filter )
    : _impl( new PipeFilter::Impl( this, filter ))
{}

PipeFilter::PipeFilter( const FilterFunc& func,
                        const PortInfos& inputPorts,
                        const PortInfos& outputPorts )
    : _impl( new PipeFilter::Impl( this,
                                   func,
                                   inputPorts,
                                   outputPorts ))
{}

PipeFilter::~PipeFilter()
{

}

FilterPtr PipeFilter::_getFilter()
{
    return _impl->_filter;
}

FilterInput& PipeFilter::_getInput()
{
    return _impl->_filterInput;
}

const FilterInput& PipeFilter::_getInput() const
{
    return _impl->_filterInput;
}

FilterOutput& PipeFilter::_getOutput()
{
    return _impl->_filterOutput;
}

const FilterOutput& PipeFilter::_getOutput() const
{
    return _impl->_filterOutput;
}

bool PipeFilter::hasInputPort( const std::string& portName ) const
{
    return _impl->hasInputPort( portName );
}

bool PipeFilter::hasOutputPort( const std::string& portName ) const
{
    return _impl->hasOutputPort( portName );
}

const servus::uint128_t& PipeFilter::getId() const
{
    return _impl->_id;
}

void PipeFilter::execute()
{
    _impl->execute();
}

void PipeFilter::waitForInput( const std::string& portName ) const
{
    _impl->waitForInput( portName );
}

void PipeFilter::waitForOutput( const std::string& portName ) const
{
    _impl->waitForOutput( portName );
}

bool PipeFilter::waitForAnyInput( lunchbox::Strings& portNames ) const
{
    return _impl->waitForAnyInput( portNames );
}

bool PipeFilter::waitForAnyOutput( lunchbox::Strings& portNames ) const
{
    return _impl->waitForAnyOutput( portNames );
}

bool PipeFilter::isOutputPortReady( const std::string& portName  ) const
{
    return _impl->isOutputPortReady( portName );
}

bool PipeFilter::isInputPortReady( const std::string& portName  ) const
{
    return _impl->isInputPortReady( portName );
}

bool PipeFilter::isInputReady() const
{
    return _impl->isInputPortReady( "" );
}

size_t PipeFilter::getInputSize( const std::string& portName ) const
{
    return _impl->getInputSize( portName );
}

}
