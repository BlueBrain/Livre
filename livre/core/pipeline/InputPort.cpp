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
    {
    }

    void connect( OutputPortPtr outputPort )
    {
        livre::connectPorts( InputPortPtr( &_inputPort, DontDeleteObject< InputPort >()),
                        outputPort );
    }

    const std::string& getName() const
    {
        return _info.portName;
    }

    bool isReady() const
    {
        BOOST_FOREACH( const ConstOutputPortPtr& port, _outputPorts )
        {
            if( !port->isReady())
                return false;
        }
        return true;
    }

    ConstPortDataPtr getDefaultValue() const
    {
        return _info.defaultData;
    }

    void wait() const
    {
        BOOST_FOREACH( const ConstOutputPortPtr& port, _outputPorts )
            port->wait();
    }

    bool waitForAny() const
    {
        return livre::waitForAny( getConnectedPorts( ));
    }

    ConstOutputPortPtrs getConnectedPorts() const
    {
        return ConstOutputPortPtrs( _outputPorts.begin(), _outputPorts.end());
    }

    size_t getSize() const
    {
        return _outputPorts.size();
    }

    InputPort& _inputPort;
    OutputPortPtrs _outputPorts;
    const PortInfo _info;
};

InputPort::InputPort( const PortInfo& portInfo )
    : _impl( new InputPort::Impl( *this, portInfo ))
{}

InputPort::~InputPort()
{

}

bool InputPort::isReady() const
{
    return _impl->isReady();
}

OutputPortPtrs& InputPort::_getConnectedPorts()
{
    return _impl->_outputPorts;
}

ConstPortDataPtr InputPort::_getDefaultValue() const
{
    return _impl->getDefaultValue();
}

void InputPort::_connect( OutputPortPtr outputPort )
{
    _impl->connect( outputPort );
}

const std::string& InputPort::getName() const
{
    return _impl->getName();
}

size_t InputPort::getSize() const
{
    return _impl->getSize();
}

void InputPort::wait() const
{
    _impl->wait();
}

bool InputPort::waitForAny() const
{
    return _impl->waitForAny();
}

ConstOutputPortPtrs InputPort::getConnectedPorts() const
{
    return _impl->getConnectedPorts();
}

void connectPorts( InputPortPtr inputPort, OutputPortPtr outputPort )
{
    if( inputPort->_getDefaultValue()->getDataType()
            != outputPort->_getDefaultValue()->getDataType())
    {
        LBTHROW( std::runtime_error( "Cannot connect ports. Types does not match"));
    }

    inputPort->_impl->_outputPorts.push_back( outputPort );
}

}
