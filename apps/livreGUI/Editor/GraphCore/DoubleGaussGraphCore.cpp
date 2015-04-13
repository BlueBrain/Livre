/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                     Philipp Schlegel <schlegel@ifi.uzh.ch>
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

#include <livre/core/Maths/Maths.h>

#include <livreGUI/Editor/GraphCore/DoubleGaussGraphCore.h>
#include <livreGUI/Editor/GraphCore/GaussGraphCore.h>

namespace livre
{

DoubleGaussGraphCore::DoubleGaussGraphCore( )
{

}

DoubleGaussGraphCore::~DoubleGaussGraphCore( )
{
}

void DoubleGaussGraphCore::initialize( TransferFunction1Df& transferFunction )
{
    const uint32_t nChannels = transferFunction.getNumChannels( );

    anchorLists_.resize( nChannels );

    for ( uint32_t i = 0; i < nChannels; ++i )
    {
        anchorLists_[ i ].clear( );
    }

    for ( uint32_t i = 0; i < nChannels - 1; ++i)
    {
        anchorLists_[ i ].push_back( Vector2f( 0.0f, 0.5f ) );
        anchorLists_[ i ].push_back( Vector2f( 0.25f, 0.5f ) );
        anchorLists_[ i ].push_back( Vector2f( 0.5f, 0.5f ) );
        anchorLists_[ i ].push_back( Vector2f( 0.75f, 0.5f ) );
    }

    anchorLists_.back( ).push_back( Vector2f( 0.0f, 1.0f ) );
    anchorLists_.back( ).push_back( Vector2f( 0.25f, 0.0f ) );
    anchorLists_.back( ).push_back( Vector2f( 0.5f, 1.0f ) );
    anchorLists_.back( ).push_back( Vector2f( 0.75f, 0.0f ) );

    AnchorGraphCore::initialize( transferFunction );
}

AnchorGraphCore::AnchorVector DoubleGaussGraphCore::getCurve( const ColorChannel channel ) const
{
    AnchorGraphCore::AnchorVector gauss1, gauss2;

    gauss1.push_back( anchorLists_[ channel ][ 0 ] );
    gauss1.push_back( anchorLists_[ channel ][ 1 ] );

    gauss2.push_back( anchorLists_[ channel ][ 2 ] );
    gauss2.push_back( anchorLists_[ channel ][ 3 ] );

    gauss1 = GaussGraphCore::transformAnchors( gauss1 );
    gauss2 = GaussGraphCore::transformAnchors( gauss2 );

    AnchorGraphCore::AnchorVector result = gauss1;
    result.push_back( gauss2.front( ) );
    result.push_back( gauss2.back( ) );

    return result;
}

void DoubleGaussGraphCore::setCurve( TransferFunction1Df& transferFunction,
                                     const ColorChannel channel,
                                     const AnchorVector& anchors)
{
    AnchorVector gauss1, gauss2;

    gauss1.push_back( anchors[ 0 ] );
    gauss1.push_back( anchors[ 1 ] );
    gauss2.push_back( anchors[ 2 ] );
    gauss2.push_back( anchors[ 3 ] );

    gauss1 = transformParameters( gauss1 );
    gauss2 = transformParameters( gauss2 );

    AnchorVector result = gauss1;
    result.push_back( gauss2.front( ) );
    result.push_back( gauss2.back( ) );

    setAnchorVector( transferFunction, channel, result );
}


void DoubleGaussGraphCore::writeCurve_( TransferFunction1Df& transferFunction ,
                                        const ColorChannel channel,
                                        const AnchorVector& anchors )
{
    if( anchors.empty( ) )
        return;

    const Vector2f& start1 = anchors.front( );
    const Vector2f& end1 = *( anchors.begin( ) + 1 );
    const Vector2f& start2 = *( anchors.end( ) - 2 );
    const Vector2f& end2 = anchors.back( );

    const float radius1 = end1.x( ) - start1.x( );
    const float radius2 = end2.x( ) - start2.x( );
    const float height1 = start1.y( ) - end1.y( );
    const float height2 = start2.y( ) - end2.y( );

    const Vector2f offset1( end1.x( ), 1.0f - start1.y( ) );
    const Vector2f offset2( end2.x( ), 1.0f - start2.y( ) );

    const float radiusFactor1 = radius1 != 0.0f ? MAGICAL_NUMBER / ( radius1 * radius1 ) : 0.0f;
    const float radiusFactor2 = radius2 != 0.0f ? MAGICAL_NUMBER / ( radius2 * radius2 ) : 0.0f;

    const uint32_t channelCount = transferFunction.getNumChannels( );
    FloatVector& data = transferFunction.getData( );
    const size_t count = data.size( ) / channelCount;

    for ( size_t i = 0; i < count; ++i )
    {
        const float x1 = static_cast< float >( i ) / ( count - 1 ) - offset1.x( );
        const float x2 = static_cast< float >( i ) / ( count - 1 ) - offset2.x( );

        float result1 = 0.0f;

        if( radiusFactor1 != 0.0f )
        {
            result1 = height1 * exp(- x1 * x1 * radiusFactor1 ) + offset1.y( );
        }
        else
        {
            result1 = ( i == start1.x( ) * (count - 1) ) ?  height1 + offset1.y( ) : offset1.y( );
        }

        float result2 = 0.0f;

        if( radiusFactor2 != 0.0f )
        {
            result2 = height2 * exp(- x2 * x2 * radiusFactor2) + offset2.y( );
        }
        else
        {
            result2 = ( i == start2.x( ) * (count - 1) ) ?  height2+ offset2.y( ) : offset2.y( );
        }

        data[ i * channelCount + channel ] = livre::maths::clamp( std::max( result1, result2 ), 0.0f, 1.0f );
    }
}

}
