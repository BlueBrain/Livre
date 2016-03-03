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

#include <livre/core/pipeline/PromiseMap.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/pipeline/Promise.h>

namespace livre
{

typedef std::pair< std::string, PromisePtr > NamePromisePair;
typedef std::map< std::string, PromisePtr > NamePromiseMap;

struct PromiseMap::Impl
{
    Impl( const Promises& promises )
    {
        for( const PromisePtr& promise: promises )
            _promiseMap[ promise->getName( )] = promise;
    }

    void throwPortError( const std::string& portName ) const
    {
        std::stringstream err;
        err << "Unknown port: " << portName << std::endl;
        LBTHROW( std::runtime_error( err.str( )));
    }

    bool hasPort( const std::string& portName ) const
    {
        return _promiseMap.count( portName ) > 0;
    }

    void flush( const std::string& portName )
    {
        if( portName != ALL_PORTS && !hasPort( portName ))
            throwPortError( portName );

        for( const auto& pair: _promiseMap )
        {
            const PromisePtr& promise = pair.second;
            if( portName == ALL_PORTS || promise->getName() == portName )
                promise->flush();
        }
    }

    PromisePtr getPromise( const std::string& portName )
    {
        if( !hasPort( portName ))
            throwPortError( portName );

        return _promiseMap[ portName ];
    }

    NamePromiseMap _promiseMap;
};

PromiseMap::PromiseMap( const Promises& promises )
    : _impl( new PromiseMap::Impl( promises ))
{}

PromiseMap::~PromiseMap()
{}

void PromiseMap::flush( const std::string& portName /* = ALL_PORTS */ )
{
    _impl->flush( portName );
}

PromisePtr PromiseMap::getPromise( const std::string& portName )
{
    return _impl->getPromise( portName );
}

}
