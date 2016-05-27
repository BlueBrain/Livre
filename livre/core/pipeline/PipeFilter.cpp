
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

#include <livre/core/pipeline/InputPort.h>
#include <livre/core/pipeline/OutputPort.h>
#include <livre/core/pipeline/PromiseMap.h>
#include <livre/core/pipeline/FutureMap.h>
#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/FuturePromise.h>
#include <livre/core/pipeline/Filter.h>

namespace livre
{

struct PipeFilter::Impl
{
    typedef std::map< std::string, OutputPort > OutputPortMap;
    typedef std::map< std::string, InputPort > InputPortMap;

    Impl( PipeFilter& pipeFilter,
          const std::string& name,
          FilterPtr filter )
        : _pipeFilter( pipeFilter )
        , _name( name )
        , _filter( std::move( filter ))
    {
        for( const DataInfo& dataInfo: _filter->getInputDataInfos( ))
        {
            _inputMap.emplace( std::piecewise_construct,
                               std::forward_as_tuple( dataInfo.first ),
                               std::forward_as_tuple( dataInfo ));
        }

        for( const DataInfo& dataInfo: _filter->getOutputDataInfos( ))
        {
            _outputMap.emplace( std::piecewise_construct,
                                std::forward_as_tuple( dataInfo.first ),
                                std::forward_as_tuple( dataInfo ));
        }
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

    void execute()
    {
        Futures inputFutures;
        for( const auto& namePort: _inputMap )
        {
            const Futures& futures = namePort.second.getFutures();
            for( const auto& future: futures )
                inputFutures.emplace_back( future, namePort.second.getName( ));
        }

        const FutureMap futures( inputFutures );
        PromiseMap promises( getOutputPromises( ));

        try
        {
            _filter->execute( futures, promises );
            promises.flush();
        }
        catch( const std::runtime_error& err )
        {
            promises.flush();
            throw err;
        }
        catch( const std::logic_error& err )
        {
            promises.flush();
            throw err;
        }
    }

    Promise getInputPromise( const std::string& portName )
    {
        if( !hasInputPort( portName ))
            throwPortError( portName );

        const auto& itPromise = _manuallySetPortsMap.find( portName );
        if( itPromise != _manuallySetPortsMap.end( ))
            return itPromise->second.getPromise();

        InputPort& inputPort = _inputMap.find( portName )->second;
        const auto& itPort =  _manuallySetPortsMap.emplace( std::piecewise_construct,
                                      std::forward_as_tuple( inputPort.getName( )),
                                      std::forward_as_tuple( DataInfo( inputPort.getName(),
                                                                       inputPort.getDataType( ))));
        OutputPort& outputPort = itPort.first->second;
        inputPort.connect( outputPort );
        return outputPort.getPromise();
    }

    Promises getOutputPromises() const
    {
        Promises promises;
        for( const auto& namePort: _outputMap )
            promises.push_back( namePort.second.getPromise( ));

        return promises;
    }

    Futures getPostconditions() const
    {
        Futures futures;
        for( const auto& namePort: _outputMap )
        {
            const Future& outputFuture = namePort.second.getPromise().getFuture();
            futures.push_back( outputFuture );
        }
        return futures;
    }

    Futures getPreconditions() const
    {
        Futures futures;
        for( const auto& namePort: _inputMap )
        {
            const Futures& inputFutures = namePort.second.getFutures();
            futures.insert( futures.end(), inputFutures.begin(), inputFutures.end( ));
        }
        return futures;
    }

    void connect( const std::string& srcPortName,
                  Impl& dstImpl,
                  const std::string& dstPortName )
    {
        if( !hasOutputPort( srcPortName ))
            throwPortError( srcPortName );

        if( !dstImpl.hasInputPort( dstPortName ))
            throwPortError( dstPortName );

        // The value on the output port may already be set
        if( !dstImpl._manuallySetPortsMap.count( dstPortName ))
            std::runtime_error( std::string( "The value on port:  ")
                                + dstPortName
                                + "is already set" );

        _outputMap.find( srcPortName )->second.connect(
                    dstImpl._inputMap.find( dstPortName )->second );
    }

    void reset()
    {
        for( auto& namePort: _manuallySetPortsMap )
            _inputMap.find( namePort.first )->second.disconnect( namePort.second );

        _manuallySetPortsMap.clear();
        for( auto& namePort: _outputMap )
            namePort.second.reset();

    }

    PipeFilter& _pipeFilter;
    const std::string _name;
    const FilterPtr _filter;
    InputPortMap _inputMap;
    OutputPortMap _outputMap;
    OutputPortMap _manuallySetPortsMap;
};

PipeFilter::PipeFilter( const std::string& name,
                        FilterPtr&& filter )
    : _impl( new Impl( *this, name, std::move( filter )))
{}

ExecutablePtr PipeFilter::clone() const
{
    return ExecutablePtr( new PipeFilter( *this ));
}

PipeFilter::~PipeFilter()
{}

std::string PipeFilter::getName() const
{
    return _impl->_name;
}

Promise PipeFilter::getPromise( const std::string& portName )
{
    return _impl->getInputPromise( portName );
}

void PipeFilter::execute()
{
    _impl->execute();
}

Futures PipeFilter::getPostconditions() const
{
    return _impl->getPostconditions();
}

Futures PipeFilter::getPreconditions() const
{
    return _impl->getPreconditions();
}

void PipeFilter::reset()
{
    _impl->reset();
}

void PipeFilter::connect( const std::string& srcPortName,
                          PipeFilter& dst,
                          const std::string& dstPortName )
{
    _impl->connect( srcPortName, *dst._impl, dstPortName );
}

}
