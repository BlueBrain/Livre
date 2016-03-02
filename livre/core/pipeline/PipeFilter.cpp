
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
#include <livre/core/pipeline/PortPromises.h>
#include <livre/core/pipeline/PortFutures.h>
#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/pipeline/PortInfo.h>
#include <livre/core/pipeline/FunctionFilter.h>
#include <livre/core/pipeline/Future.h>
#include <livre/core/pipeline/Promise.h>

namespace livre
{

struct PipeFilter::Impl
{
    Impl( PipeFilter& pipeFilter,
          const std::string& name,
          FilterPtr filter )
        : _pipeFilter( pipeFilter )
        , _name( name )
        , _id( servus::make_UUID( ))
        , _filter( filter )
    {
        addPorts( filter->getInputPorts(),
                  filter->getOutputPorts( ));
    }

    Impl( PipeFilter& pipeFilter,
          const std::string& name,
          const FilterFunc& filterFunc,
          const PortInfos& inputPorts,
          const PortInfos& outputPorts )
        : _pipeFilter( pipeFilter )
        , _name( name )
        , _id( servus::make_UUID( ))
        , _filter( new FunctionFilter( filterFunc,
                                       inputPorts,
                                       outputPorts ))
    {
        addPorts( inputPorts, outputPorts );
    }


    bool hasInputPort( const std::string& portName ) const
    {
        return _inputMap.count( portName ) > 0;
    }

    bool hasOutputPort( const std::string& portName ) const
    {
        return _outputMap.count( portName ) > 0;
    }

    void throwPortError( const std::string& portName ) const
    {
        LBTHROW( std::runtime_error( std::string( "There is no port with name: ")
                                                  + portName ));
    }

    void addPorts( const PortInfos& iPortInfos, const PortInfos& oPortInfos )
    {
        for( const PortInfo& portInfo: iPortInfos )
            _inputMap[ portInfo.name ] = new InputPort( portInfo );

        for( const PortInfo& portInfo: oPortInfos )
            _outputMap[ portInfo.name ] = new OutputPort( *this, portInfo );

        // Connect notification ports
         _inputMap[ _id.getString( )] =
                 new InputPort( makePortInfo< ConstPortDataPtr >( _id.getString( )));
         _outputMap[ _id.getString( )] =
                 new OutputPort( *this, makePortInfo< ConstPortDataPtr >( _id.getString( )));
    }

    void execute()
    {
        InputPorts ports;
        for( auto pair: _inputMap )
            ports.push_back( pair.second );

        const InFutures futures( ports );
        PortPromises promises( getOutputPromises( ));
        _filter->execute( futures, promises );
        promises.flush();
    }

    PromisePtr getInputPromise( const std::string& portName ) const
    {
        if( !hasInputPort( portName ))
            throwPortError( portName );

        if( _outputsToInputMap.count( portName ) > 0 )
            return _outputsToInputMap[ portName ]->getPromise();

        InputPortPtr& inputPort = _inputMap[ portName ];

        if(  inputPort->getSize() != 0 || // If there is another connection
            inputPort->getName() == _id.getString( )) // If this is an notification port
        {
            LBTHROW( std::runtime_error( "The port is already connected or this is a"
                                         "notification port" ));
        }

        OutputPortPtr outputPort( new OutputPort( *this,
                                      makePortInfo( inputPort->getName(),
                                                    inputPort->getDataType( ))));
        _outputsToInputMap[ inputPort->getName() ] = outputPort;

        return outputPort->getPromise();
    }

    Promises getOutputPromises() const
    {
        Promises promises;
        promises.reserve( _outputPorts.size( ));
        for( const auto& pair: _outputMap )
            promises.push_back( pair.second->getPromise( ));

        return promises;
    }

    Futures getOutFutures() const
    {
        for( const auto& pair: _outputPorts )
        {
            ConstFuturePtr outputFuture = pair.second->getPromise()->getFuture();
            futures.push_back( outputFuture );
        }
        return futures;
    }

    Futures getInputFutures( lunchbox::Strings& names ) const
    {
        Futures futures;
        for( const auto& pair: _inputPorts )
        {
            Futures inputFutures = port->getFutures();
            futures.insert( futures.end(), inputFutures.begin(), inputFutures.end( ));
            names.push_back( pair.second->getName( ));
        }
        return futures;
    }

    void connect( const std::string& srcPortName,
                  PipeFilterPtr dst,
                  const std::string& dstPortName )
    {
        if( !hasOutputPort( srcPortName ))
            throwPortError( srcPortName );

        if( !dst->_impl.hasInputPort( dstPortName ))
            throwPortError( dstPortName );

        // The value on the output port may already be set
        if( !dst->_impl._outputsToInputMap.count( dstPortName ))
            std::runtime_error( std::string( "The value on port:  ")
                                + portName
                                + "is already set" );

        _outputMap[ srcPortName ]->connect( *dst->_impl->inputMap[ dstPortName ] );
    }

    void PipeFilter::connect( PipeFilterPtr dst )
    {
        OutputPortPtr outputPort = _outputMap[ _id.getString() ];
        InputPortPtr inputPort = dst->_impl->_inputMap[ dst->_impl->_id.getString() ];
        inputPort->connect( *outputPort );
    }

    PipeFilter& _pipeFilter;
    const std::string _name;
    const servus::uint128_t _id;
    FilterPtr _filter;
    InputPortMap _inputMap;
    OutputPortMap _outputMap;
    OutputPortMap _outputsToInputMap;
};

PipeFilter::PipeFilter( const std::string& name,
                        FilterPtr filter )
    : _impl( new PipeFilter::Impl( *this, name, filter ))
    , Executable( _impl->_promise )
{}

PipeFilter::PipeFilter( const std::string& name,
                        const FilterFunc& func,
                        const PortInfos& inputPorts,
                        const PortInfos& outputPorts )
    : _impl( new PipeFilter::Impl( *this,
                                   name,
                                   func,
                                   inputPorts,
                                   outputPorts ))
    , Executable( _filterOutput.getPorts( ))
{}

PipeFilter::~PipeFilter()
{}

void PipeFilter::execute()
{
    _impl->execute();
}

const servus::uint128_t& PipeFilter::getId() const
{
    return _impl->_id;
}

const std::string& PipeFilter::getName() const
{
    return _impl->_name;
}

Futures PipeFilter::getConnectedInFutures() const
{
    lunchbox::Strings& names;
    return _impl->getInputFutures( names );
}

PromisePtr PipeFilter::getPromise( const std::string& portName )
{
    return _impl->getInputPromise( portName );
}

Futures PipeFilter::getOutFutures() const
{
    return _impl->getOutFutures();
}

void PipeFilter::connect( const std::string& srcPortName,
                          PipeFilterPtr dst,
                          const std::string& dst )
{
     _impl->connect( srcPortName, dst, dst );
}

void PipeFilter::connect( PipeFilterPtr dst )
{
    _impl->connect( dst );
}

}
