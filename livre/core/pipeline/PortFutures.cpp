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

#include <livre/core/pipeline/PortFutures.h>
#include <livre/core/pipeline/Future.h>

namespace livre
{

typedef std::multimap< std::string, ConstFuturePtr > NameFutureMap;
typedef std::pair< std::string, ConstFuturePtr > NameFuturePair;

struct PortFutures::Impl
{
    void throwError( const std::string& portName ) const
    {
        LBTHROW( std::runtime_error( std::string( "No futures assigned with the given port with name: ")
                                                  + portName ));
    }

    bool hasPort( const std::string& portName ) const
    {
        return _futureMap.count( portName ) > 0;
    }

    ConstFutures getFutures( const std::string& portName ) const
    {
        if( portName == ALL_PORTS )
        {
            ConstFutures futures;
            futures.insert( futures.end(), _futureMap.begin(), _futureMap.end( ));
            return futures;
        }

        if( !hasPort( portName ))
            throwError( portName );

        ConstFutures futures;
        NameFutureMap::const_iterator it = _futureMap.find( portName );
        futures.insert( futures.end(), it, _futureMap.end( ));
        return futures;
    }


    bool areReady()
    {
        for( const auto& future: getFutures( portName ))
        {
            if( !future->isReady())
                return false;
        }
        return true;
    }

    void wait()
    {
        for( const auto& future: getFutures( portName ))
            future->wait();
    }

    void waitForAny( const std::string& portName ) const
    {
        return livre::waitForAny( getFutures( portName ));
    }

    void addFuture( const std::string& name, const ConstFuturePtr& future )
    {
        _futureMap[ name ] = future;
    }

    NameFutureMap _futureMap;
};


PortFutures::PortFutures( const ConstFutures& futures,
                          const lunchbox::Strings& portNames /* = lunchbox::Strings() */)
    : _impl( new PortFutures::Impl( futures, portNames ))
{}

PortFutures::~PortFutures()
{}

size_t PortFutures::getInputSize( const std::string& portName ) const
{
    return _impl->getPortSize( portName );
}

ConstFutures PortFutures::getFutures( const std::string& portName ) const
{
    return _impl->getFutures( portName );
}

bool PortFutures::areReady( const std::string& portName ) const
{
    return _impl->areReady( portName );
}

void PortFutures::wait( const std::string& portName ) const
{
    return _impl->wait( portName );
}

void PortFutures::waitForAny( const std::string& portName ) const
{
    return _impl->waitForAny( portName );
}

PortFutures::PortFutures()
{}

void PortFutures::_addFuture( const std::string& name, const ConstFuturePtr& future )
{
    _impl->addFuture( name, future );
}

OutputPortFutures::OutputPortFutures( const ConstFutures& futures )
{
    for( const auto& future: futures )
        _addFuture( future->getName(), future );

}

OutputPortFutures::~OutputPortFutures()
{}

InputPortFutures::InputPortFutures( const InputPorts& inputPorts )
{
     for( const auto& inputPort: inputPorts )
     {
         const ConstFutures& futures = inputPort->getFutures();
         for( const auto& future: futures )
             _addFuture( future->getName(), future );
     }
}

InputPortFutures::~InputPortFutures()
{}


}
