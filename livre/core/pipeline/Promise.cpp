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

#include <livre/core/pipeline/Promise.h>
#include <livre/core/pipeline/Future.h>

namespace livre
{

struct Promise::Impl
{
    Impl( const PipeFilter& pipeFilter, const AsyncData& data )
        : _pipeFilter( pipeFilter )
        , _data( data )
        , _future( _pipeFilter, _data )
    {}

    const std::string& getName() const
    {
        return _data.getName();
    }

    void set( ConstPortDataPtr msg )
    {
        _data.set( msg );
    }

    Future _future;
    const PipeFilter& _pipeFilter;
    const AsyncData& _data;
};

Promise::Promise( const PipeFilter& pipeFilter,
                  const AsyncData& data )
    : _impl( new Promise::Impl( pipeFilter, data ))
{}

Promise::~Promise()
{}

const std::string& Promise::getName() const
{
    return _impl->getName();
}

void Promise::set( ConstPortDataPtr msg )
{
    _impl->getName( msg );
}

Future Promise::getFuture() const
{
    return _impl->_future;
}

}
