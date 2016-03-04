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
#include <livre/core/pipeline/Promise.h>
#include <livre/core/pipeline/Future.h>

namespace livre
{

struct InputPort::Impl
{
    Impl( const PortInfo& info )
        : _info( info )
    {}

    ~Impl()
    {}

    const std::string& getName() const
    {
        return _info.name;
    }

    size_t getSize() const
    {
        return _futures.size();
    }

    const std::type_index& getDataType() const
    {
        return _info.getDataType();
    }

    void connect( const OutputPort& outputPort )
    {
        if( getDataType() != outputPort.getDataType( ))
            LBTHROW( std::runtime_error( "Data types does not match between ports"));

        _futures.push_back( outputPort.getPromise()->getFuture( ));
    }

    void disconnect( const OutputPort& outputPort )
    {
        Futures::iterator it = _futures.begin();
        while( it != _futures.end())
        {
            if( *it == outputPort.getPromise()->getFuture( ))
            {
                _futures.erase( it );
                return;
            }
            ++it;
        }
    }

    Futures _futures;
    const PortInfo _info;
};

InputPort::InputPort( const PortInfo& portInfo )
    : _impl( new InputPort::Impl( portInfo ))
{}

InputPort::~InputPort()
{}

const Futures& InputPort::getFutures() const
{
    return _impl->_futures;
}

void InputPort::connect( const OutputPort& outputPort )
{
    _impl->connect( outputPort );
}

void InputPort::disconnect( const OutputPort& outputPort )
{
    _impl->disconnect( outputPort );
}

const std::string& InputPort::getName() const
{
    return _impl->getName();
}

const std::type_index& InputPort::getDataType() const
{
    return _impl->getDataType();
}

size_t InputPort::getSize() const
{
    return _impl->getSize();
}


}
