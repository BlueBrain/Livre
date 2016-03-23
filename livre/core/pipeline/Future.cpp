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
#include <livre/core/pipeline/AsyncData.h>

namespace livre
{

struct Future::Impl
{
    Impl( const AsyncData& data )
        : _name( data.getName( ))
        , _data( data )
    {}

    Future rename( const std::string& name ) const
    {
        Future ret( _data );
        ret._impl->_name = name;
        return ret;
    }

    std::string getName() const
    {
        return _name;
    }

    PortDataPtr get( const std::type_index& dataType ) const
    {
        return _data.get( dataType );
    }

    bool isReady() const
    {
        return _data.isReady();
    }

    void wait() const
    {
        return _data.wait();
    }

    std::string _name;
    const AsyncData& _data;
};

Future::Future( const AsyncData& data )
    : _impl( new Future::Impl( data ))
{}

Future::~Future()
{}

std::string Future::getName() const
{
    return _impl->getName();
}

Future::Future( const Future& future, const std::string& name )
    : _impl( future._impl )
{
    _impl->_name = name;
}

void Future::wait() const
{
    return _impl->wait();
}

bool Future::isReady() const
{
    return _impl->isReady();
}

bool Future::operator==( const Future& future ) const
{
    return &_impl->_data == &future._impl->_data;
}

const AsyncData& Future::_getAsyncData() const
{
    return _impl->_data;
}

PortDataPtr Future::_getPtr( const std::type_index& dataType ) const
{
    return _impl->get( dataType );
}

}
