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

#include <livre/core/pipeline/Promise.h>
#include <livre/core/pipeline/Future.h>
#include <livre/core/pipeline/OutputPort.h>

namespace livre
{

struct Promise::Impl
{
    Impl( const PipeFilter& pipeFilter, AsyncData& data )
        : _pipeFilter( pipeFilter )
        , _data( data )
        , _future( _pipeFilter, _data )
    {}

    const std::string& getName() const
    {
        return _data.getName();
    }

    void set( const ConstPortDataPtr& data )
    {
        _data.set( data );
    }

    void flush()
    {
        _data.set( ConstPortDataPtr( ));
    }

    const PipeFilter& _pipeFilter;
    AsyncData& _data;
    const Future _future;
};

Promise::Promise( const PipeFilter& pipeFilter,
                  AsyncData& data )
    : _impl( new Promise::Impl( pipeFilter, data ))
{}

Promise::~Promise()
{}

const std::string& Promise::getName() const
{
    return _impl->getName();
}

void Promise::flush()
{
    _impl->flush();
}

const Future& Promise::getFuture() const
{
    return _impl->_future;
}

void Promise::_set( const ConstPortDataPtr& data )
{
    _impl->set( data );
}

}
