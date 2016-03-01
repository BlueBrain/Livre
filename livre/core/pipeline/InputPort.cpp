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

namespace livre
{

struct InputPort::Impl
{
    Impl( InputPort& inputPort,
          const PortInfo& info )
        : _inputPort( inputPort )
        , _info( info )
    {}

    ~Impl()
    {}

    const std::string& getName() const
    {
        return _info.portName;
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
        if( getDataType() != outputPort->getDataType( ))
            LB_THROW( std::exception( "Data types does not match between ports"));

        _futures.push_back( outputPort.getPromise()-getFuture( ));
    }

    ConstFutures _futures;
    const PortInfo _info;
};

InputPort::InputPort( const PortInfo& portInfo )
    : _impl( new InputPort::Impl(  portInfo ))
{}

InputPort::~InputPort()
{}

ConstFutures InputPort::getFutures() const
{
    return _impl->getFutures();
}

void InputPort::connect( const OutputPort& outputPort )
{
    _impl->connect( outputPort );
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
