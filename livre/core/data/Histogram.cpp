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

Histogram::Histogram()
{
    setMin( std::numeric_limits<float>::infinity( ));
    setMax( -std::numeric_limits<float>::infinity( ));
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

    static_cast< ::lexis::render::Histogram& >( *this ) = histogram;
    return *this;
}

Histogram& Histogram::operator+=( const Histogram& histogram )
{
    if( histogram.getBins().size() != getBins().size() ||
        histogram.getMin() != getMin() ||
        histogram.getMax() != getMax( ))
    {
        LBTHROW( std::runtime_error( "Addition of incompatible histograms"));
    }

    const uint64_t* srcBins = histogram.getBins().data();

    uint64_t* bins = getBins().data();
    for( size_t i = 0; i < getBins().size(); ++i )
        bins[ i ] += srcBins[ i ];

    return *this;
}

bool Histogram::isDataUniform( size_t& uniformIndex ) const
{
    const uint64_t* bins = getBins().data();
    size_t nonZeroCount = 0;
    for( size_t i = 0; i < getBins().size(); ++i )
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
    const uint64_t* bins = getBins().data();
    return std::distance( bins, std::min_element( bins, bins + getBins().size( )));
}

size_t Histogram::getMaxIndex() const
{
    const uint64_t* bins = getBins().data();
    return std::distance( bins, std::max_element( bins, bins + getBins().size( )));
}

bool Histogram::isEmpty() const
{
    const uint64_t* bins = getBins().data();
    for( size_t i = 0; i < getBins().size(); ++i )
    {
        if( bins[ i ] > 0 )
            return false;
    }
    return true;
}

uint64_t Histogram::getSum() const
{
    const uint64_t* bins = getBins().data();
    uint64_t sum = 0;
    for( size_t i = 0; i < getBins().size(); ++i )
        sum +=  bins[ i ];

    return sum;
}

Vector2f Histogram::getRange() const
{
    return Vector2f( getMin(), getMax( ));
}

double Histogram::getRatio( const size_t index ) const
{
    if( index >= getBins().size( ))
        return 0.0f;

    const uint64_t sum = getSum();
    if( sum == 0 )
        return 0.0f;

    const uint64_t* bins = getBins().data();
    return (double)bins[ index ] / (double)sum;
}

void Histogram::resize( size_t newSize )
{
    getBins().clear();
    for( size_t i = 0; i < newSize; ++i )
    {
        getBins().push_back( 0 );
    }
}

std::ostream& operator<<( std::ostream& os, const Histogram& histogram )
{
    os << " Min value index: " << histogram.getMinIndex()
       << " Max value index: " << histogram.getMaxIndex()
       << std::endl;

    const uint64_t* data = histogram.getBins().data();
    for( size_t i = 0; i < histogram.getBins().size(); ++i )
        os << data[ i ] << " ";
    os << std::endl;
    return os;
}

}
