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

#include <livre/core/pipeline/FilterInput.h>
#include <livre/core/pipeline/InputPort.h>
#include <livre/core/pipeline/OutputPort.h>

namespace livre
{

typedef std::pair< std::string, InputPortPtr > NameInputPortPair;
typedef std::map< std::string, InputPortPtr > InputPortMap;

struct FilterInput::Impl
{
    Impl()
    {}

    void throwError( const std::string& portName ) const
    {
        LBTHROW( std::runtime_error( std::string( "No input port with name: ")
                                                  + portName ));
    }

    InputPortPtr getPort( const std::string& portName )
    {
        if(!hasPort( portName ))
            throwError( portName );

        return _portMap[ portName ];
    }

    ConstInputPortPtr getPort( const std::string& portName ) const
    {
        if(!hasPort( portName ))
            throwError( portName );

        return _portMap.find( portName )->second;
    }

    InputPortPtr addPort( const PortInfo& portInfo )
    {
        if( hasPort( portInfo.portName ))
            LBTHROW( std::runtime_error( std::string( "Exitsting port name: " ) +
                                         portInfo.portName ));

        InputPortPtr port( new InputPort( portInfo ));
        _portMap.insert( std::make_pair( portInfo.portName, port ));

        return port;
    }

    void wait( const std::string& portName ) const
    {
        if( !portName.empty())
        {
            getPort( portName )->wait();
            return;
        }

        BOOST_FOREACH( const NameInputPortPair& namePortPair, _portMap )
        {
            ConstInputPortPtr port = namePortPair.second;
            port->wait();
        }
    }

    bool waitForAny( lunchbox::Strings& ports ) const
    {
        ConstInputPortPtrs readyPorts;
        ConstInputPortPtrs notReadyPorts;

        BOOST_FOREACH( const NameInputPortPair& namePortPair, _portMap )
        {
            const ConstInputPortPtr& port = namePortPair.second;
            if( !port->isReady( ))
            {
                readyPorts.push_back( port );
                ports.push_back( port->getName( ));
            }
            else
                notReadyPorts.push_back( port );
        }

        if( notReadyPorts.empty( ))
            return false;

        ConstOutputPortPtrs outputs;
        BOOST_FOREACH( ConstInputPortPtr& port, notReadyPorts )
        {
            if( !port->isReady( ))
            {
                const ConstOutputPortPtrs& outputPorts = port->getConnectedPorts();
                outputs.insert( outputs.end(),
                                outputPorts.begin(),
                                outputPorts.end());
            }
        }

        livre::waitForAny( outputs );
        return true;
    }

    bool waitForAny( const std::string& portName ) const
    {
        return getPort( portName )->waitForAny();
    }

    lunchbox::Strings getPortNames() const
    {
        lunchbox::Strings names;
        BOOST_FOREACH( const NameInputPortPair& nameInputPortPair,
                       _portMap )
            names.push_back( nameInputPortPair.first );
        return names;
    }

    bool hasPort( const std::string& portName ) const
    {
        return _portMap.count( portName ) > 0;
    }

    bool isReady( const std::string& portName = "" ) const
    {
        if( !portName.empty( ))
            return getPort( portName )->isReady();

        BOOST_FOREACH( const NameInputPortPair& namePortPair,
                       _portMap )
        {
            const ConstInputPortPtr& port = namePortPair.second;
            if( !port->isReady( ))
                return false;
        }

        return true;
    }

    size_t getPortSize( const std::string& portName ) const
    {
        return getPort( portName )->getSize();
    }

    void clear()
    {
        _portMap.clear();
    }

    bool isEmpty() const
    {
        return _portMap.empty();
    }

    InputPortMap _portMap;
};


FilterInput::FilterInput()
    : _impl( new FilterInput::Impl( ))
{}

FilterInput::~FilterInput()
{

}

size_t FilterInput::getInputSize( const std::string& portName ) const
{
    return _impl->getPortSize( portName );
}

ConstInputPortPtr FilterInput::getPort( const std::string& portName ) const
{
    return _impl->getPort( portName );
}

bool FilterInput::waitForAny( const std::string& portName ) const
{
    return _impl->waitForAny( portName );
}

bool FilterInput::waitForAny( lunchbox::Strings& readyPorts ) const
{
    return _impl->waitForAny( readyPorts );
}

bool FilterInput::isReady(const std::string& portName) const
{
    return _impl->isReady( portName );
}

void FilterInput::wait( const std::string& portName ) const
{
    _impl->wait( portName );
}

lunchbox::Strings FilterInput::getPortNames() const
{
    return _impl->getPortNames();
}

bool FilterInput::hasPort( const std::string& portName ) const
{
    return _impl->hasPort( portName );
}

void FilterInput::clear()
{
    _impl->clear();
}

bool FilterInput::isEmpty() const
{
    return _impl->isEmpty();
}

InputPortPtr FilterInput::getPort( const std::string& portName )
{
    return _impl->getPort( portName );
}

InputPortPtr FilterInput::addPort( const PortInfo& portInfo )
{
    return _impl->addPort( portInfo );
}

}
