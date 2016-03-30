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
#include <livre/core/pipeline/Promise.h>

namespace livre
{

namespace
{

const std::string ALL_PROMISES = "ALL_PROMISES";

}

typedef std::map< std::string, Promise > NamePromiseMap;
typedef NamePromiseMap::value_type NamePromisePair;

struct PromiseMap::Impl
{
    Impl( const Promises& promises )
    {
        for( const auto& promise: promises )
            _promiseMap.insert({ promise.getName(), promise });
    }

    void throwError( const std::string& name ) const
    {
        std::stringstream err;
        err << "Unknown promise name: " << name << std::endl;
        LBTHROW( std::runtime_error( err.str( )));
    }

    bool hasPromise( const std::string& name ) const
    {
        return _promiseMap.count( name ) > 0;
    }

    void flush( const std::string& name ) const
    {
        getPromise( name ).flush();
    }

    void flush() const
    {
        for( auto& namePromise: _promiseMap )
        {
            Promise promise = namePromise.second;
            promise.flush();
        }
    }

    Promise getPromise( const std::string& name ) const
    {
        if( !hasPromise( name ))
            throwError( name );

        return _promiseMap.find( name )->second;
    }

    NamePromiseMap _promiseMap;
};

PromiseMap::PromiseMap( const Promises& promises )
    : _impl( new PromiseMap::Impl( promises ))
{}

PromiseMap::~PromiseMap()
{}

void PromiseMap::flush( const std::string& name ) const
{
    _impl->flush( name );
}

void PromiseMap::flush() const
{
     _impl->flush();
}

Promise PromiseMap::getPromise( const std::string& name ) const
{
    return _impl->getPromise( name );
}

}
