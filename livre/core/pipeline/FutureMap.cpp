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

#include <livre/core/pipeline/FutureMap.h>
#include <livre/core/pipeline/InputPort.h>
#include <livre/core/pipeline/OutputPort.h>

namespace livre
{

typedef std::multimap< std::string, Future > NameFutureMap;
typedef std::pair< std::string, Future > NameFuturePair;

struct FutureMapImpl
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
            for( const auto& pair: _futureMap )
                futures.push_back( pair.second );
            return futures;
        }

        if( !hasPort( portName ))
            throwPortError( portName );

        Futures futures;
        NameFutureMap::const_iterator it = _futureMap.find( portName );

        while( it != _futureMap.end( ))
        {
            futures.push_back( it->second );
            ++it;
        }

        return futures;
    }

    bool isReady( const std::string& portName ) const
    {
        for( const auto& future: getFutures( portName ))
        {
            if( !future.isReady())
                return false;
        }
        return true;
    }

    void wait( const std::string& portName ) const
    {
        for( const auto& future: getFutures( portName ))
            future.wait();
    }

    bool waitForAny( const std::string& portName ) const
    {
        return livre::waitForAny( getFutures( portName ));
    }

    void addFuture( const std::string& name, const Future& future )
    {
        _futureMap.insert( std::make_pair( name, future ));
    }

    NameFutureMap _futureMap;
};

struct FutureMap::Impl: public FutureMapImpl
{
public:
    Impl( const Futures& futures )
    {
        for( const auto& future: futures )
        {
            const std::string& name = future.getName();
            addFuture( name, future );
        }
    }
};

FutureMap::FutureMap( const Futures& futures )
    : _impl( new FutureMap::Impl( futures ))
{}

Futures FutureMap::getFutures() const
{
    return _impl->getFutures( ALL_PORTS );
}

bool FutureMap::isReady() const
{
    return _impl->isReady( ALL_PORTS );
}

void FutureMap::wait() const
{
    _impl->wait( ALL_PORTS );
}

bool FutureMap::waitForAny() const
{
    return _impl->waitForAny( ALL_PORTS );
}

FutureMap::~FutureMap()
{}

struct OutFutureMap::Impl: public FutureMapImpl
{
public:
    Impl( const Futures& futures )
    {
        for( const auto& future: futures )
        {
            const std::string& name = future.getName();
            if( hasPort( name ))
                throwPortError( name );
            addFuture( name, future );
        }
    }
};

OutFutureMap::OutFutureMap( const Futures& futures )
    : _impl( new OutFutureMap::Impl( futures ))
{}

Futures OutFutureMap::getFutures() const
{
    return _impl->getFutures( ALL_PORTS );
}

Future OutFutureMap::getFuture( const std::string& portName ) const
{
    return _impl->getFutures( portName ).front();
}

bool OutFutureMap::isReady( const std::string& portName ) const
{
    return _impl->isReady( portName );
}

void OutFutureMap::wait( const std::string& portName ) const
{
    _impl->wait( portName );
}

bool OutFutureMap::waitForAny() const
{
    return _impl->waitForAny( ALL_PORTS );
}

OutFutureMap::~OutFutureMap()
{}

struct InFutureMap::Impl: public FutureMapImpl
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

InFutureMap::InFutureMap( const InputPorts& inputPorts )
    : _impl( new InFutureMap::Impl( inputPorts ))
{
}

Futures InFutureMap::getFutures( const std::string& portName ) const
{
    return _impl->getFutures( portName );
}

bool InFutureMap::isReady( const std::string& portName ) const
{
    return _impl->isReady( portName );
}

void InFutureMap::wait( const std::string& portName ) const
{
    _impl->wait( portName );
}

bool InFutureMap::waitForAny( const std::string& portName ) const
{
    return _impl->waitForAny( portName );
}

InFutureMap::~InFutureMap()
{}



}
