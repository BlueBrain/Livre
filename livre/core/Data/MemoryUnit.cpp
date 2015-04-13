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

#include <livre/core/Data/MemoryUnit.h>

namespace livre
{

MemoryUnit::MemoryUnit()
{}

MemoryUnit::~MemoryUnit()
{
    // release(); is virtual and can't be called from dtor
}

ConstMemoryUnit::ConstMemoryUnit( const uint8_t* ptr, const uint32_t size )
    : ptr_( ptr ), size_( size )
{
}

uint32_t ConstMemoryUnit::getMemSize() const
{
    return size_;
}

const uint8_t* ConstMemoryUnit::getData_() const
{
    return ptr_;
}


AllocMemoryUnit::AllocMemoryUnit()
{}

uint32_t AllocMemoryUnit::getMemSize() const
{
    return _rawData.getSize();
}

uint32_t AllocMemoryUnit::getAllocSize() const
{
    return _rawData.getMaxSize();
}

void AllocMemoryUnit::alloc( const uint32_t allocSizePerElement,
                             const uint32_t dataSize )
{
    LB_TS_THREAD( thread_ );
    _rawData.resize( allocSizePerElement * dataSize );
}

void AllocMemoryUnit::release()
{
    _rawData.clear();
}

const uint8_t* AllocMemoryUnit::getData_() const
{
    return _rawData.getData();
}

uint8_t* AllocMemoryUnit::getData_()
{
    return _rawData.getData();
}

}
