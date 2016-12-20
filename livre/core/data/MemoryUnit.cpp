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

#include <livre/core/data/MemoryUnit.h>

namespace livre
{

MemoryUnit::MemoryUnit()
{}

MemoryUnit::~MemoryUnit()
{}

ConstMemoryUnit::ConstMemoryUnit( const uint8_t* ptr )
    : ptr_( ptr )
{
}

const uint8_t* ConstMemoryUnit::_getData() const
{
    return ptr_;
}

size_t AllocMemoryUnit::getAllocSize() const
{
    return _rawData.getMaxSize();
}

AllocMemoryUnit::~AllocMemoryUnit()
{
    _rawData.clear();
}

void AllocMemoryUnit::_alloc( const size_t nBytes )
{
    LB_TS_THREAD( thread_ );
    _rawData.reset( nBytes );
}

const uint8_t* AllocMemoryUnit::_getData() const
{
    return _rawData.getData();
}

uint8_t* AllocMemoryUnit::_getData()
{
    return _rawData.getData();
}

}
