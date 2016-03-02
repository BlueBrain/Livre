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

#include <livre/core/pipeline/PortPromises.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/pipeline/Promise.h>

namespace livre
{

typedef std::pair< std::string, PromisePtr > NamePromisePair;
typedef std::map< std::string, PromisePtr > NamePromiseMap;

struct PortPromises::Impl
{
    Impl( const Promises& promises )
    {
        for( PromisePtr& promise: promises )
            _promiseMap[ promise->getName( )] = promise;
    }

    void throwPortError( const std::string& portName ) const
    {
        std::stringstream err;
        err << "Unknown port: " << portName << std::endl;
        _promiseMap( std::runtime_error( err.str( )));
    }

    bool hasPort( const std::string& portName ) const
    {
        return _promiseMap.count( portName ) > 0;
    }

    void flush( const std::string& portName )
    {
        if( portName != ALL_PORTS && !hasPort( portName ))
            throwPortError( portName );

        for( const NamePromisePair& namePromisePair: _promiseMap )
        {
            PromisePtr& promise = namePromisePair.second;
            if( portName == ALL_PORTS || promise->getName() == portName )
                promise->flush();
        }
    }

    void set( const std::string& portName, ConstPortDataPtr data )
    {
        if( !hasPort( portName ))
            throwPortError( portName );

        _promiseMap[ portName ]->set( data );
    }

    NamePromiseMap _promiseMap;
};

PortPromises::PortPromises( const Promises& promises )
    : _impl( new PortPromises::Impl( promises ))
{}

PortPromises::~PipeFilterOutput()
{}

void PortPromises::flush( const std::string& portName /* = ALL_PORTS */ )
{
    _impl->flush( portName );
}

void PortPromises::_set( const std::string& name,
                         ConstPortDataPtr data )
{
    _impl->set( name, data );
}


}
