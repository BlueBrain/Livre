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

#include <livre/core/pipeline/OutputPort.h>
#include <livre/core/pipeline/InputPort.h>
#include <livre/core/pipeline/Promise.h>

namespace livre
{

struct OutputPort::Impl
{
    Impl( const DataInfo& dataInfo )
        : _promise( dataInfo )
    {}

    ~Impl()
    {
       flush();
    }

    void flush()
    {
        _promise.flush();
    }

    std::type_index getDataType() const
    {
        return _promise.getDataType();
    }

    std::string getName() const
    {
        return _promise.getName();
    }

    void reset()
    {
        _promise.reset();
    }

    Promise _promise;
};

OutputPort::OutputPort( const DataInfo& dataInfo )
    : _impl( new OutputPort::Impl( dataInfo ))
{}


OutputPort::~OutputPort()
{}

std::string OutputPort::getName() const
{
    return _impl->getName();
}

std::type_index OutputPort::getDataType() const
{
    return _impl->getDataType();
}

Promise OutputPort::getPromise() const
{
    return _impl->_promise;
}

void OutputPort::connect( InputPort& port )
{
    port.connect( *this );
}

void OutputPort::reset()
{
    _impl->reset();
}

}
