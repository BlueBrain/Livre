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

#ifndef _MemoryUnit_h_
#define _MemoryUnit_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <lunchbox/buffer.h> // member
#include <lunchbox/thread.h> // LB_TS_VAR

namespace livre
{

/**
 * The MemoryUnit class the base class for carrying memory information.
 */
class MemoryUnit
{
public:
    LIVRECORE_API MemoryUnit();
    LIVRECORE_API virtual ~MemoryUnit();

    /** @return The memory ptr in type T. */
    template< class T > const T* getData() const
        { return reinterpret_cast< const T* >( _getData( )); }

    /** @return The memory ptr in type T. */
    template< class T > T* getData()
        { return reinterpret_cast< T* >( _getData( )); }

    /** @return The allocated heap size. */
    virtual size_t getAllocSize() const = 0;

protected:
    /** @return The unsigned char memory ptr to data */
    virtual const uint8_t* _getData() const = 0;

    /** @return The unsigned char memory ptr to data */
    virtual uint8_t* _getData() = 0;
};

/** MemoryUnit holding no data at all */
class NoMemoryUnit : public MemoryUnit
{
public:
    ~NoMemoryUnit() {}
    size_t getAllocSize() const final { return 0; }

private:
    const uint8_t* _getData() const final { LBDONTCALL; return 0; }
    uint8_t* _getData() final { LBDONTCALL; return 0; }
};

/**
 * The ConstMemoryUnit class shows a arbitrary memory pointer. No allocation is preformed.
 * i.e: Memory mapped files are managed by the OS and only a handle to the memory is kept.
 */
class ConstMemoryUnit : public MemoryUnit
{
public:
    explicit ConstMemoryUnit( const uint8_t* ptr );
    ~ConstMemoryUnit() {}
protected:
    size_t getAllocSize() const final { return 0; }
    const uint8_t* _getData() const final;
    uint8_t* _getData() final { LBDONTCALL; return 0; }
    const uint8_t* const ptr_;
};

/**
 * The AllocMemoryUnit class shows an allocated memory pointer to keep track of memory consumption.
 * Memory is cleaned on destruction.
 */
class AllocMemoryUnit : public MemoryUnit
{
public:

    /**
     * Allocate and copy the data from the given source and size
     * @param sourceData Source data ptr.
     * @param size Number of the elements in the source data ptr.
     */
    template< typename T >
    AllocMemoryUnit( const T* sourceData, const size_t size )
    {
        _allocAndSetData( sourceData, size );
    }

    /** "void" specialization of the constructor */
    LIVRECORE_API AllocMemoryUnit( const void* sourceData, const size_t size )
    {
        _allocAndSetData( static_cast< const uint8_t* >( sourceData ), size );
    }

    /**
     * Allocate and copy the data from the given source and size
     * @param sourceData Source data vector.
     */
    template< typename T >
    explicit AllocMemoryUnit( const std::vector< T >& sourceData )
    {
        _allocAndSetData( &sourceData[ 0 ], sourceData.size( ));
    }

    /**
     * Allocates memory in bytes in given size
     * @param size memory size
     */
    LIVRECORE_API explicit AllocMemoryUnit( const size_t size )
    {
        _alloc( size );
    }

    LIVRECORE_API ~AllocMemoryUnit();
    LIVRECORE_API size_t getAllocSize() const final;

private:

    AllocMemoryUnit( const AllocMemoryUnit& ) = delete;
    AllocMemoryUnit& operator=( const AllocMemoryUnit& ) = delete;

    template< class T >
    void _allocAndSetData( const T* sourceData, const size_t size )
    {
        _alloc( sizeof( T ) * size );
        ::memcpy( _rawData.getData(), sourceData, size * sizeof( T ) );
    }

    void _alloc( size_t nBytes );

    const uint8_t* _getData() const final;
    uint8_t* _getData() final;

    lunchbox::Bufferb _rawData;
    LB_TS_VAR( thread_ );
};

}

#endif // _MemoryUnit_h_
