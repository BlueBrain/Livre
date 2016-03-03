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

#include "PortType.h"

namespace livre
{

/**
 * PortData class is base class for keeping the track for types
 * by using the std::type_index.
 */
class PortData : public PortType
{
protected:
    explicit PortData( const std::type_index& dataType )
        : PortType( dataType) {}
    virtual ~PortData() {}
};

/**
 * Holds the T typed data.
 */
template< class T>
struct PortDataT final : public PortData
{
    explicit PortDataT( const T& data_ )
        : PortData( std::type_index( typeid( T )))
        , data( data_ )
    {}

    explicit PortDataT( const T&& data_ )
        : PortData( std::type_index( typeid( T )))
        , data( std::move( data_ ))
    {}

    ~PortDataT() {}
    const T data;

    PortDataT( const PortDataT< T >& ) = delete;
    PortDataT< T >& operator=( const PortDataT< T >& ) = delete;
};

template <class T>
using PortDataTPtr = boost::shared_ptr< PortDataT< T >>;

template <class T>
using ConstPortDataTPtr = boost::shared_ptr< PortDataT< T >>;

/**
 * Constructs a PortDataTPtr object from type T
 */
template< class T >
inline PortDataTPtr< T > makePortDataPtr( const T& data )
{
    return boost::make_shared< PortDataT< T >>( data );
}

/**
 * Constructs a PortDataTPtr object from type T.
 * Moves the to the port data.
 */
template< class T >
inline PortDataTPtr< T > makePortDataPtr( const T&& data )
{
    return boost::make_shared< PortDataT< T >>( data );
}

}

#endif // _PortData_h_

