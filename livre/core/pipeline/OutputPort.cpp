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

#include <livre/core/pipeline/OutputPort.h>
#include <livre/core/pipeline/PortInfo.h>
#include <livre/core/pipeline/Promise.h>

#include <boost/thread/future.hpp>

namespace livre
{


typedef boost::shared_future< ConstPortDataPtr > ConstPortDataFuture;
typedef boost::promise< ConstPortDataPtr > ConstPortDataPromise;
typedef std::vector< ConstPortDataFuture > ConstPortDataFutures;

struct AsyncData::Impl
{
    Impl( const std::string name )
        : _future( _promise.get_future( ))
        , _name( name )
    {}

    ~Impl()
    {
        if( !isReady())
            _promise.set_value( result );
    }

    ConstPortDataPtr get() const
    {
        return _future.get();
    }

    void set( const ConstPortDataPtr& data )
    {
        if( result )
        {
            if( data->getDataType() != _info->getDataType( ))
                LBTHROW( std::runtime_error( "Types does not match on set value"));
        }

        if( !isReady())
            _promise.set_value( result );
    }

    bool isReady() const
    {
        return _future.is_ready();
    }

    void wait() const
    {
        _future.wait();
    }

    const PipeFilter& _pipeFilter;
    ConstPortDataPromise _promise;
    mutable ConstPortDataFuture _future;
    const std::string _name;
};

AsyncData::AsyncData( const std::string& name )
    :  _impl( new Impl( pipeFilter, name ))
{}

ConstPortDataPtr AsyncData::get() const
{
    return _impl->get();
}

const std::string& AsyncData::getName() const
{
    return _impl->_name;
}

void AsyncData::set( const ConstPortDataPtr& data )
{
    _impl->set( data );
}

bool AsyncData::isReady() const
{
    return _impl->isReady();
}

void AsyncData::wait() const
{
    _impl->wait();
}


struct OutputPort::Impl
{
    Impl( const PipeFilter& pipeFilter, const PortInfo& portInfo )
        : _info( portInfo )
        , _data( portInfo.name )
        , _portPromise( new Promise( pipeFilter, _data ))
    {}

    ~Impl()
    {
       flush();
    }

    void flush()
    {
        _data.set( ConstPortDataPtr( ));
    }

    const std::string& getName() const
    {
        return _info.portName;
    }

    PortInfo _info;
    AsyncData _data;
    PromisePtr _portPromise;
};

OutputPort::OutputPort( const PipeFilter& pipeFilter, const PortInfo& portInfo )
    : _impl( new OutputPort::Impl( pipeFilter, portInfo ))
{}


OutputPort::~OutputPort()
{}

const std::string& OutputPort::getName() const
{
    return _impl->getName();
}

const std::type_index& OutputPort::getDataType() const
{
    return _impl->_info.getDataType();
}

ConstFuturePtr OutputPort::getFuture() const
{
    return _impl->_portFuture;
}

PromisePtr OutputPort::getPromise() const
{
    return _impl->_portPromise;
}

void OutputPort::connect( InputPort& inputPort )
{
    _inputPort->connect( *this );
}

bool waitForAny( const Futures& futures )
{
    ConstPortDataFutures futures;
    for( const auto& future: futures )
    {
        if( !future->isReady())
           futures.push_back( future->getAsyncData()._impl->_future );
    }

    if( futures.empty( ))
        return false;

    boost::wait_for_any( futures.begin(), futures.end( ));
    return true;
}

}
