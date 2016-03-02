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

class FutureMap
{
public:

    void throwPortError( const std::string& portName ) const
    {
        LBTHROW( std::runtime_error( std::string( "No futures assigned with the given port with name: ")
                                                  + portName ));
    }

    bool hasPort( const std::string& portName ) const
    {
        return _futureMap.count( portName ) > 0;
    }

    Futures getFutures( const std::string& portName ) const
    {
        if( portName == ALL_PORTS )
        {
            Futures futures;
            futures.insert( futures.end(), _futureMap.begin(), _futureMap.end( ));
            return futures;
        }

        if( !hasPort( portName ))
            throwPortError( portName );

        Futures futures;
        NameFutureMap::const_iterator it = _futureMap.find( portName );
        futures.insert( futures.end(), it, _futureMap.end( ));
        return futures;
    }

    bool isReady() const
    {
        for( const auto& future: getFutures( portName ))
        {
            if( !future->isReady())
                return false;
        }
        return true;
    }

    void wait( const std::string& portName ) const
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

struct OutFutures::Impl: public FutureMap
{
public:
    Impl( const Futures& futures )
    {
        for( const auto& future: futures )
        {
            const std::string& name = future->getName();
            if( hasPort( name ))
                throwPortError( name );
            addFuture( name, future );
        }
    }
};

OutFutures::OutFutures( const Futures& futures )
    : _impl( new OutFutures::Impl( futures ))
{}

Futures OutFutures::getFutures() const
{
    return _impl->getFutures( ALL_PORTS );
}

Future OutFutures::getFuture( const std::string& portName ) const
{
    return _impl->getFuture();
}

bool OutFutures::isReady( const std::string& portName ) const
{
    return _impl->isReady( portName );
}

void OutFutures::wait( const std::string& portName ) const
{
    _impl->wait( portName );
}

void OutFutures::waitForAny( const std::string& portName ) const
{
    return _impl->waitForAny( portName );
}

OutFutures::~OutFutures()
{}

struct InFutures::Impl: public FutureMap
{
public:
    Impl( const InputPorts& inputPorts )
    {
        for( const InputPortPtr& inputPort: inputPorts )
        {
            const Futures& futures = inputPort->getFutures();
            for( const auto& future: futures )
                addFuture( inputPort->getName(), future );
        }
    }
};

InFutures::InFutures( const InputPorts& inputPorts )
    : _impl( new InFutures::Impl( inputPorts ))
{
}

Futures InFutures::getFutures( const std::string& portName /* = ALL_PORTS */ ) const
{
    return _impl->getFutures( portName );
}

bool InFutures::isReady( const std::string& portName ) const
{
    return _impl->isReady( portName );
}

void InFutures::wait( const std::string& portName ) const
{
    _impl->wait( portName );
}

void InFutures::waitForAny( const std::string& portName ) const
{
    return _impl->waitForAny( portName );
}

InFutures::~InFutures()
{}



}
