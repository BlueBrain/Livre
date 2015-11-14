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

#define BOOST_THREAD_PROVIDES_FUTURE

#include <boost/thread/future.hpp>
#include <boost/function.hpp>

namespace livre
{

typedef boost::shared_future< ConstPortDataPtr > ConstPortDataFuture;
typedef boost::promise< ConstPortDataPtr > ConstPortDataPromise;

typedef std::vector< ConstPortDataFuture > ConstPortDataFutures;
typedef std::vector< ConstPortDataPromise > ConstPortDataPromises;


struct OutputPort::Impl
{

    Impl( const PortInfo& portInfo )
        : _info( portInfo )
        , _future( _promise.get_future( ))
    {
    }

    ~Impl()
    {
       flush();
    }

    const std::string& getName() const
    {
        return _info.portName;
    }

    ConstPortDataPtr getValue() const
    {
        return _future.get();
    }

    ConstPortDataPtr getDefaultValue() const
    {
        return _info.defaultData;
    }

    void setValue( ConstPortDataPtr result )
    {
        if( result )
        {
            if( result->getDataType() != _info.defaultData->getDataType( ))
                LBTHROW( std::runtime_error( "Types does not match on set value"));
        }

        if( !isReady())
            _promise.set_value( result );
    }

    void flush()
    {
       setValue( _info.defaultData );
    }

    bool isReady() const
    {
        return _future.is_ready();
    }

    void wait() const
    {
        _future.wait();
    }

    PortInfo _info;
    std::string _name;
    ConstPortDataPromise _promise;
    mutable ConstPortDataFuture _future;
};

OutputPort::OutputPort( const PortInfo& portInfo )
    : _impl( new OutputPort::Impl( portInfo ))
{}


OutputPort::~OutputPort()
{

}

const std::string& OutputPort::getName() const
{
    return _impl->getName();
}

void OutputPort::flush()
{
    _impl->flush();
}

bool OutputPort::isReady() const
{
    return _impl->isReady();
}

ConstPortDataPtr OutputPort::_getValue() const
{
    return _impl->getValue();
}

void OutputPort::_setValue( ConstPortDataPtr msg )
{
    _impl->setValue( msg );
}

ConstPortDataPtr OutputPort::_getDefaultValue() const
{
    return _impl->getDefaultValue();
}

void OutputPort::wait() const
{
    _impl->wait();
}

bool waitForAny( const ConstOutputPortPtrs& ports )
{
    ConstPortDataFutures futures;
    BOOST_FOREACH( const ConstOutputPortPtr& port, ports )
    {
        if( !port->isReady())
            futures.push_back( port->_impl->_future );
    }

    if( futures.empty( ))
        return false;

    boost::wait_for_any( futures.begin(), futures.end( ));
    return true;
}

void waitForAll( const ConstOutputPortPtrs& ports )
{
    BOOST_FOREACH( const ConstOutputPortPtr& port, ports )
        port->wait();
}

}
