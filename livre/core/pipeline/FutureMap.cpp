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

typedef std::multimap< std::string, Future > NameFutureMap;
typedef std::pair< std::string, Future > NameFuturePair;

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
        NameFutureMap::const_iterator it = _futureMap.find( name );

        while( it != _futureMap.end( ))
        {
            futures.push_back( it->second );
            ++it;
        }

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

    bool waitForAny( const std::string& name ) const
    {
        return livre::waitForAny( getFutures( name ));
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
    if( name == ALL_FUTURES )
        LBTHROW( std::runtime_error( "All futures cannot be retrieved with this function"))

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

bool UniqueFutureMap::waitForAny() const
{
    return _impl->waitForAny( ALL_FUTURES );
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

bool FutureMap::isReady( const std::string& name ) const
{
    return _impl->isReady( name );
}

void FutureMap::wait( const std::string& name ) const
{
    _impl->wait( name );
}

bool FutureMap::waitForAny( const std::string& name ) const
{
    return _impl->waitForAny( name );
}

FutureMap::~FutureMap()
{}



}
