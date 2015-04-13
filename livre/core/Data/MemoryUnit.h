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

#include <livre/core/types.h>
#include <lunchbox/buffer.h> // member
#include <lunchbox/thread.h> // LB_TS_VAR
#include <boost/noncopyable.hpp>

namespace livre
{

/**
 * The MemoryUnit class the base class for carrying memory information.
 */
class MemoryUnit
{
public:
    MemoryUnit();

    virtual ~MemoryUnit();

    /**
     * Allocate memory.
     * @param allocSizePerElement Allocation size per element of the array.
     * @param dataSize Number of elements in the array.
     */
    virtual void alloc( const uint32_t allocSizePerElement LB_UNUSED,
                        const uint32_t dataSize LB_UNUSED ) = 0;

    /** Release memory */
    virtual void release() = 0;

    /** @return The memory ptr in type T. */
    template< class T > const T* getData() const
        { return reinterpret_cast< const T* >( getData_( )); }

    /** @return The memory ptr in type T. */
    template< class T > T* getData()
        { return reinterpret_cast< T* >( getData_( )); }

    /**
     * @return The size of the memory accessed.
     */
    virtual uint32_t getMemSize() const = 0;

    /** @return The allocated heap size. */
    virtual uint32_t getAllocSize() const = 0;

protected:
    /** @return The unsigned char memory ptr to data */
    virtual const uint8_t* getData_() const = 0;

    /** @return The unsigned char memory ptr to data */
    virtual uint8_t* getData_() = 0;
};

/** MemoryUnit holding no data at all */
class NoMemoryUnit : public MemoryUnit
{
public:
    virtual ~NoMemoryUnit() {}
    void alloc( const uint32_t, const uint32_t ) final { LBDONTCALL; }
    void release() final {}

    uint32_t getMemSize() const final { return 0; }
    uint32_t getAllocSize() const final { return 0; }

private:
    const uint8_t* getData_() const final { LBDONTCALL; return 0; }
    uint8_t* getData_() final { LBDONTCALL; return 0; }
};

/**
 * The ConstMemoryUnit class shows a memory pointer that is anywhere in memory. No allocation is present.
 */
class ConstMemoryUnit : public MemoryUnit
{
public:
    ConstMemoryUnit( const uint8_t* ptr, const uint32_t size );

protected:
    uint32_t getMemSize() const final;
    uint32_t getAllocSize() const final { return getMemSize(); }
    const uint8_t* getData_() const final;
    uint8_t* getData_() final { LBDONTCALL; return 0; }

    void alloc( const uint32_t, const uint32_t ) final { LBDONTCALL; }
    void release() final {}

    const uint8_t* const ptr_;
    const uint32_t size_;
};

/**
 * The AllocMemoryUnit class shows an allocated memory pointer to keep track of memory consumption.
 */
class AllocMemoryUnit : public MemoryUnit, public boost::noncopyable
{
public:
    AllocMemoryUnit();

    /**
     * Allocates and sets the data from the given ptr, with given size
     * @param sourceData Source data ptr.
     * @param size Number of the elements in the source data ptr.
     */
    template< class T >
    void allocAndSetData( const T* sourceData, const uint32_t size )
    {
        alloc( sizeof( T ), size );
        ::memcpy( _rawData.getData(), sourceData, size * sizeof( T ) );
    }

    /**
     * Allocates and sets the data from the given source data vector, with given size
     * @param sourceData Source data vector.
     */
    template< class T >
    void allocAndSetData( const std::vector< T >& sourceData )
    {
        allocAndSetData( &sourceData[ 0 ], sourceData.size());
    }

    /**
     * Allocates memory.
     * @param allocSizePerElement Allocation size per element of the array.
     * @param dataSize Number of elements in the array.
     */
    void alloc( const uint32_t allocSizePerElement, const uint32_t dataSize )
        final;

    /** Release memory. */
    void release() final;

    uint32_t getMemSize() const final;
    uint32_t getAllocSize() const final;

private:
    const uint8_t* getData_() const final;
    uint8_t* getData_() final;

    LB_TS_VAR( thread_ );
    lunchbox::Bufferb _rawData;
};

}

#endif // _MemoryUnit_h_
