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

#ifndef _PortData_h_
#define _PortData_h_

#include <livre/core/types.h>

namespace livre
{

/**
 * Base class for keeping the track for types of data
 * by using the std::type_index.
 */
class PortData
{
public:
    const std::type_index dataType;

protected:
    explicit PortData( const std::type_index& dataType_ )
        : dataType( dataType_ ) {}
    virtual ~PortData() {}
};

/**
 * Holds the T typed data.
 */
template< class T>
struct PortDataT final : public PortData
{
    /**
     * Constructor
     * @param data_ is copied
     */
    explicit PortDataT( const T& data_ )
        : PortData( getType< T >())
        , data( data_ )
    {}

    ~PortDataT() {}
    const T data;

    PortDataT( const PortDataT< T >& ) = delete;
    PortDataT( PortDataT< T >&& ) = delete;
    PortDataT< T >& operator=( const PortDataT< T >& ) = delete;
};

}

#endif // _PortData_h_

