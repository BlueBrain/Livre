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

#include <livre/core/pipeline/Future.h>

#include <livre/core/pipeline/OutputPort.h>

namespace livre
{

struct Future::Impl
{
    Impl( const PipeFilter& pipeFilter,
          const AsyncData& data )
        : _pipeFilter( pipeFilter )
        , _data( data )
    {}

    const std::string& getName() const
    {
        return _data.getName();
    }

    ConstPortDataPtr get() const
    {
        return _data.get();
    }

    bool isReady() const
    {
        return _data.isReady();
    }

    void wait() const
    {
        return _data.wait();
    }

    const PipeFilter& _pipeFilter;
    const AsyncData& _data;
};

Future::Future( const PipeFilter& pipeFilter,
                const AsyncData& data )
    : _impl( new Future::Impl( pipeFilter, data ))
{}

Future::~Future()
{}

const AsyncData& Future::getAsyncData() const
{
    return _impl->_data;
}

const std::string& Future::getName() const
{
    return _impl->getName();
}

void Future::wait() const
{
    return _impl->wait();
}

bool Future::isReady() const
{
    return _impl->isReady();
}

const PipeFilter& Future::getPipeFilter() const
{
    return _impl->_pipeFilter;
}

ConstPortDataPtr Future::_get() const
{
    return _impl->get();
}

}
