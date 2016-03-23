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
#include <livre/core/pipeline/Future.h>
#include <livre/core/pipeline/AsyncData.h>

namespace livre
{

namespace
{

const std::string ALL_FUTURES = "ALL_FUTURES";

}

typedef std::multimap< std::string, Future > NameFutureMap;

struct FutureMapImpl
{
public:

    void throwError( const std::string& name ) const
    {
        LBTHROW( std::runtime_error( std::string( "Unknown future name: ")
                                                  + name ));
    }

    bool hasFuture( const std::string& name ) const
    {
        return _futureMap.count( name ) > 0;
    }

    Futures getFutures( const std::string& name ) const
    {
        if( name == ALL_FUTURES )
        {
            Futures futures;
            for( const auto& pair: _futureMap )
                futures.push_back( pair.second );
            return futures;
        }

        if( !hasFuture( name ))
            throwError( name );

        Futures futures;
        const auto& itPair = _futureMap.equal_range( name );

        for( auto it = itPair.first; it != itPair.second; ++it )
            futures.push_back( it->second );

        return futures;
    }

    bool isReady( const std::string& name ) const
    {
        for( const auto& future: getFutures( name ))
        {
            if( !future.isReady())
                return false;
        }
        return true;
    }

    void wait( const std::string& name ) const
    {
        for( const auto& future: getFutures( name ))
            future.wait();
    }

    void waitForAny( const std::string& name ) const
    {
        livre::waitForAny( getFutures( name ));
    }

    void addFuture( const std::string& name, const Future& future )
    {
        _futureMap.insert( std::make_pair( name, future ));
    }

    NameFutureMap _futureMap;
};

struct UniqueFutureMap::Impl: public FutureMapImpl
{
public:
    Impl( const Futures& futures )
    {
        for( const auto& future: futures )
        {
            const std::string& name = future.getName();
            if( hasFuture( name ))
                throwError( name );
            addFuture( name, future );
        }
    }
};

UniqueFutureMap::UniqueFutureMap( const Futures& futures )
    : _impl( new UniqueFutureMap::Impl( futures ))
{}

Futures UniqueFutureMap::getFutures() const
{
    return _impl->getFutures( ALL_FUTURES );
}

Future UniqueFutureMap::getFuture( const std::string& name ) const
{
    return _impl->getFutures( name ).front();
}

bool UniqueFutureMap::isReady( const std::string& name ) const
{
    return _impl->isReady( name );
}

void UniqueFutureMap::wait( const std::string& name ) const
{
    _impl->wait( name );
}

void UniqueFutureMap::waitForAny() const
{
    _impl->waitForAny( ALL_FUTURES );
}

UniqueFutureMap::~UniqueFutureMap()
{}

struct FutureMap::Impl: public FutureMapImpl
{
public:
    Impl( const Futures& futures )
    {
        for( const auto& future: futures )
            addFuture( future.getName(), future );

    }
};

FutureMap::FutureMap( const Futures& futures )
    : _impl( new FutureMap::Impl( futures ))
{
}

Futures FutureMap::getFutures( const std::string& name ) const
{
    return _impl->getFutures( name );
}

Futures FutureMap::getFutures() const
{
    return _impl->getFutures( ALL_FUTURES );
}

bool FutureMap::isReady( const std::string& name ) const
{
    return _impl->isReady( name );
}

bool FutureMap::isReady() const
{
    return _impl->isReady( ALL_FUTURES );
}

void FutureMap::wait( const std::string& name ) const
{
    _impl->wait( name );
}

void FutureMap::wait() const
{
    _impl->wait( ALL_FUTURES );
}

void FutureMap::waitForAny( const std::string& name ) const
{
    _impl->waitForAny( name );
}

void FutureMap::waitForAny() const
{
    _impl->waitForAny( ALL_FUTURES );
}

FutureMap::~FutureMap()
{}



}
