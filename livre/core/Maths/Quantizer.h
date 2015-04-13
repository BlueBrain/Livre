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

#ifndef _Quantizer_h_
#define _Quantizer_h_

#include <climits>
#include <livre/core/mathTypes.h>

namespace livre
{

/**
 * Quantizes an array of type T to type U between min and max
 * @param srcData Source data pointer.
 * @param dstData Destination data pointer.
 * @param count Number of elements in source data.
 * @param compCount Component count for source data.
 * @param min Minimum value of source data.
 * @param max Maximum value of source data.
 */
template < class T, class U >
void unsignedQuantize( const T* srcData,
                       U* dstData,
                       const uint32_t count,
                       const uint32_t compCount,
                       const Vector3f& min,
                       const Vector3f& max )
{
    const float dataTypeMax = std::numeric_limits< U >::max( );
    const Vector3f range = max - min;

    #pragma omp parallel for schedule( dynamic, 65536 )
    for( uint32_t i = 0; i < count; ++i )
    {
        for( uint32_t j = 0; j < compCount; ++j )
        {
            dstData[ i + j ] = ( (float)srcData[ i + j ] - min[ j ] ) / range[ j ] * dataTypeMax;
        }
    }
}

/**
 * Quantizes an array of type T to type U between min and max
 * @param srcData Source data pointer.
 * @param dstData Destination data pointer.
 * @param count Number of elements in source data.
 * @param compCount Component count for source data.
 * @param min Minimum value of source data.
 * @param max Maximum value of source data.
 */
template < class T, class U >
void signedQuantize( const T* srcData,
                     U* dstData,
                     const uint32_t count,
                     const uint32_t compCount,
                     const Vector3f& min,
                     const Vector3f& max )
{
    const float dataTypeMax = std::numeric_limits< U >::max( );
    const Vector3f range = max - min;

    #pragma omp parallel for schedule( dynamic, 65536 )
    for( uint32_t i = 0; i < count; ++i )
    {
        for( uint32_t j = 0; j < compCount; ++j )
        {
            dstData[ i + j ] = 2.0 * ( ( (float)srcData[ i + j ] - min[ j ] ) / range[ j ] - 0.5 ) * dataTypeMax;
        }
    }
}

}

#endif // _Quantizer_h_
