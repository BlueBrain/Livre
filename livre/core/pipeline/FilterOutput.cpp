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

#include <livre/core/pipeline/FilterOutput.h>
#include <livre/core/pipeline/OutputPort.h>
#include <livre/core/pipeline/PortData.h>

namespace livre
{

typedef std::pair< std::string, OutputPortPtr > NameOutputPortPair;
typedef std::map< std::string, OutputPortPtr > OutputPortMap;

struct FilterOutput::Impl
{
    Impl()
    {
    }

    void throwError( const std::string& portName ) const
    {
        std::stringstream err;
        err << "Unknown port: " << portName << std::endl;
        LBTHROW( std::runtime_error( err.str( )));
    }

    Strings getPortNames() const
    {
        lunchbox::Strings names;
        BOOST_FOREACH( const NameOutputPortPair& namePortPair,
                       _portMap )
        {
            names.push_back( namePortPair.first );
        }
        return names;
    }

    OutputPortPtr getPort( const std::string& portName )
    {
        if(!hasPort( portName ))
            throwError( portName );

        return _portMap[ portName ];
    }

    ConstOutputPortPtrs getPorts() const
    {
        ConstOutputPortPtrs outputPorts;
        BOOST_FOREACH( const NameOutputPortPair& namePortPair,
                       _portMap )
        {
            outputPorts.push_back( namePortPair.second );
        }
        return outputPorts;
    }

    ConstOutputPortPtr getPort( const std::string& portName ) const
    {
        if(!hasPort( portName ))
            throwError( portName );

        return _portMap.find( portName )->second;
    }

    OutputPortPtr addPort( const PortInfo& portInfo )
    {
        if( _portMap.count( portInfo.portName ) == 0 )
        {
            OutputPortPtr port( new OutputPort( portInfo ));
            _portMap.insert( std::make_pair( portInfo.portName, port ));
        }
        return _portMap[ portInfo.portName ];
    }

    bool hasPort( const std::string& portName ) const
    {
        if( _portMap.count( portName ) > 0 )
            return true;

        return false;
    }

    void flush()
    {
        BOOST_FOREACH( const NameOutputPortPair& namePortPair,
                       _portMap )
        {
            OutputPortPtr port = namePortPair.second;
            port->flush();
        }
    }

    bool waitForAny( lunchbox::Strings& readyPorts ) const
    {
        ConstOutputPortPtrs notReadyPorts;
        BOOST_FOREACH( const NameOutputPortPair& namePortPair,
                       _portMap )
        {
            const ConstOutputPortPtr& port = namePortPair.second;
            const std::string& portName = port->getName( );
            if( isReady( portName ))
                readyPorts.push_back( portName );
            else
                notReadyPorts.push_back( port );
        }

        if( notReadyPorts.empty( ))
            return false;

        livre::waitForAny( notReadyPorts );
        return true;
    }

    void wait( const std::string& portName = "" ) const
    {
        if( !portName.empty())
        {
            getPort( portName )->wait();
            return;
        }

        BOOST_FOREACH( const NameOutputPortPair& namePortPair,
                       _portMap )
        {
            namePortPair.second->wait();
        }
    }

    bool isReady( const std::string& portName ) const
    {
        if( !portName.empty())
            return getPort( portName )->isReady();

        BOOST_FOREACH( const NameOutputPortPair& namePortPair,
                       _portMap )
        {
            const ConstOutputPortPtr& port = namePortPair.second;
            if( !port->isReady( ))
                return false;
        }

        return true;
    }

    OutputPortMap _portMap;
};

FilterOutput::FilterOutput( )
    : _impl( new FilterOutput::Impl())
{}

FilterOutput::~FilterOutput()
{
    
}

void FilterOutput::flush()
{
    _impl->flush();
}

OutputPortPtr FilterOutput::getPort( const std::string& portName )
{
    return _impl->getPort( portName  );
}

OutputPortPtr FilterOutput::addPort( const PortInfo& portInfo )
{
    return _impl->addPort( portInfo );
}

ConstOutputPortPtrs FilterOutput::getPorts() const
{
    return _impl->getPorts();
}

ConstOutputPortPtr FilterOutput::getPort( const std::string& portName ) const
{
    return _impl->getPort( portName );
}

bool FilterOutput::hasPort( const std::string& portName ) const
{
    return _impl->hasPort( portName );
}

Strings FilterOutput::getPortNames() const
{
    return _impl->getPortNames();
}

bool FilterOutput::waitForAny( Strings& readyPorts ) const
{
    return _impl->waitForAny( readyPorts );
}

void FilterOutput::wait( const std::string& portName ) const
{
    return _impl->wait( portName );
}

bool FilterOutput::isReady( const std::string& portName ) const
{
    return _impl->isReady( portName );
}

}
