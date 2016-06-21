/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <livre/core/data/Histogram.h>

namespace livre
{

const size_t Histogram::binCount = 256;

Histogram::Histogram()
{
    std::memset( getBins(), 0, sizeof( uint64_t ) * binCount );
}

Histogram::Histogram( const Histogram& histogram )
    : co::Distributable< ::lexis::render::Histogram >( histogram )
{
    if( this == &histogram )
        return;

    *this = histogram;
}

Histogram& Histogram::operator=( const Histogram& histogram )
{
    if( this == &histogram )
        return *this;

    const uint64_t* srcBins = histogram.getBins();
    uint64_t* bins = getBins();
    std::memcpy( bins, srcBins, sizeof( uint64_t) * binCount );
    return *this;
}

Histogram& Histogram::operator+=( const Histogram& histogram )
{
    const uint64_t* srcBins = histogram.getBins();

    uint64_t* bins = getBins();
    for( size_t i = 0; i < binCount; ++i )
        bins[ i ] += srcBins[ i ];

    return *this;
}

bool Histogram::isUniform( size_t& uniformIndex ) const
{
    const uint64_t* bins = getBins();
    size_t nonZeroCount = 0;
    for( size_t i = 0; i < binCount; ++i )
    {
        if( bins[ i ] > 0 )
        {
            uniformIndex = i;
            ++nonZeroCount;
        }

        if( nonZeroCount > 1 )
            break;
    }
    return nonZeroCount == 1 ;
}

size_t Histogram::getMinIndex() const
{
    const uint64_t* bins = getBins();
    return std::distance( bins, std::min_element( bins, bins + binCount ));
}

size_t Histogram::getMaxIndex() const
{
    const uint64_t* bins = getBins();
    return std::distance( bins, std::max_element( bins, bins + binCount ));
}

std::ostream& operator<<( std::ostream& os, const Histogram& histogram )
{
    os << " Min value index: " << histogram.getMinIndex()
       << " Max value index: " << histogram.getMaxIndex()
       << std::endl;

    const uint64_t* data = histogram.getBins();
    for( size_t i = 0; i < Histogram::binCount; ++i )
        os << data[ i ] << " ";
    os << std::endl;
    return os;
}

}
