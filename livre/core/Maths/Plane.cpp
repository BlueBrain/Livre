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

#include <livre/core/Maths/Plane.h>

namespace livre
{

Plane::Plane( )
    : a_( 0.0f ), b_( 0.0f ), c_( 0.0f ), d_( 0.0f )
{ }

Plane::Plane( const Vector4f& params )
{
    set( params[ 0 ], params[ 1 ], params[ 2 ], params[ 3 ] );
}

Plane::Plane( float a, float b, float c, float d )
{
    set( a, b, c, d );
}

Plane Plane::translate( float distanceVal ) const
{
    Plane plane = *this;
    plane.d_ = plane.d_ + distanceVal;
    return plane;
}

float Plane::distance( const Vector3f& pnt ) const
{
    return a_ * pnt[0] + b_ * pnt[1] + c_ * pnt[2] + d_;
}

float Plane::getd( ) const
{
    return d_;
}

void Plane::normalize( )
{
    float invLen = 1.0 / sqrt( a_*a_ + b_*b_ + c_*c_ );
    a_ *= invLen; b_ *= invLen; c_ *= invLen; d_ *= invLen;
}

Vector3f Plane::getNormal( ) const
{
    return Vector3f( a_, b_, c_ );
}

Vector3f Plane::getAbsNormal( ) const
{
    return Vector3f( std::abs( a_ ), std::abs( b_ ), std::abs( c_ ) );
}

void Plane::set( const float a, const float b, const float c, const float d )
{
    a_ = a; b_ = b; c_ = c; d_ = d;
    normalize( );
}

Vector4f Plane::getPlaneCoeffs() const { return Vector4f( a_, b_, c_, d_ ); }

#undef far
#undef near

void Plane::getNearFarPoints( const Boxf& bb,
                              Vector3f& near,
                              Vector3f& far) const
{
    const Vector3f& normal = getNormal();
    const Vector3f& minPos = bb.getMin();
    const Vector3f& maxPos = bb.getMax();

    for( size_t i = 0; i < 3; ++i )
    {
        if( normal[ i ] >= 0.0 )
        {
            near[ i ] = minPos[ i ];
            far[ i ] = maxPos[ i ];
        }
        else
        {
            near[ i ] = maxPos[ i ];
            far[ i ] = minPos[ i ];
        }
    }
}

}
