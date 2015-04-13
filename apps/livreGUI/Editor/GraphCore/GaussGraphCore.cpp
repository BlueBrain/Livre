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

#include <livreGUI/Editor/GraphCore/GaussGraphCore.h>

namespace livre
{

GaussGraphCore::GaussGraphCore( )
{
}

GaussGraphCore::~GaussGraphCore( )
{
}

void GaussGraphCore::initialize( TransferFunction1Df& transferFunction )
{
    anchorLists_.resize( transferFunction.getNumChannels( ) );

    for( uint32_t i = 0; i < transferFunction.getNumChannels( ); ++i )
    {
        anchorLists_[i].clear( );
    }

    for( uint32_t i = 0; i < transferFunction.getNumChannels( ); ++i)
    {
        anchorLists_[ i ].push_back( Vector2f( 1.0f / 3.0f, 0.5f ) );
        anchorLists_[ i ].push_back( Vector2f( 0.5f, 0.5f ) );
    }

    anchorLists_.back( ).push_back( Vector2f( 1.0f / 3.0f, 1.0f ) );
    anchorLists_.back( ).push_back( Vector2f( 0.5f, 0.0f ) );

   AnchorGraphCore::initialize( transferFunction );
}

AnchorGraphCore::AnchorVector GaussGraphCore::transformAnchors( const AnchorGraphCore::AnchorVector &_anchors )
{
    GaussGraphCore::AnchorVector anchors( _anchors.size( ) );
    Vector2f& lastDestAnchor = anchors.back( );

    const Vector2f& firstAnchor = _anchors.front( );
    const Vector2f& lastAnchor = _anchors.back( );

    lastDestAnchor = Vector2f( lastAnchor.y( ) - firstAnchor.y( ), firstAnchor.y( ) );

    // 1 / 256 because this is the smallest distinct distance.
    const float start = lastAnchor.x( );
    const float end = std::abs( ( start - firstAnchor.x( ) ) /
                                std::sqrt( -std::log( 1.0f / ( 256.0f * lastDestAnchor.x( ) ) ) ) );

    Vector2f& firstDestAnchor = anchors.front( );
    firstDestAnchor = Vector2f( end, start );

    return anchors;
}

AnchorGraphCore::AnchorVector GaussGraphCore::transformParameters( const ParameterVector& parameters )
{
    GaussGraphCore::AnchorVector anchors( parameters.size( ) );
    const Vector2f& firstAnchor = parameters.front( );
    const Vector2f& lastAnchor = parameters.back( );

    const float a = lastAnchor.x( );
    const float b = firstAnchor.y( );
    const float c = firstAnchor.x( );
    const float d = lastAnchor.y( );

    // 1 / 256 because this is the smallest distinct distance
    const float discriminant = a != 0.0f ? c * std::sqrt( -std::log( 1.0f / ( 256.0f * std::abs( a ) ) ) ) : b;

    Vector2f& firstDestAnchor = anchors.front( );
    Vector2f& lastDestAnchor = anchors.back( );

    firstDestAnchor = Vector2f( b - discriminant > (1.0f - b - discriminant) ? b - discriminant : b + discriminant, d );
    lastDestAnchor = Vector2f( b, a + d );

    return anchors;
}

void GaussGraphCore::setAnchor( TransferFunction1Df& transferFunction,
                                const ColorChannel channel,
                                const uint32_t anchorIndex,
                                const Vector2f& _anchor )
{
    Vector2f anchor( _anchor.x( ), 1.0f - _anchor.y( ) );
    anchorLists_[ channel ][ anchorIndex ] = anchor;
    updateCurve_( transferFunction, channel );
}

void GaussGraphCore::writeCurve_( TransferFunction1Df& transferFunction,
                                  const ColorChannel channel,
                                  const AnchorVector &anchors )
{
    if( anchors.empty( ) )
        return;

    const Vector2f& start = anchors.front( );
    const Vector2f& end = anchors.back( );

    FloatVector& data = transferFunction.getData( );

    const float radius = end.x( ) - start.x( );
    const float height = start.y( ) - end.y( );
    const Vector2f offset( end.x( ), 1.0f - start.y( ) );
    const uint32_t channelCount = transferFunction.getNumChannels( );
    const size_t count = data.size( ) / channelCount;
    if (radius != 0.0f)
    {
        const float radiusFactor = MAGICAL_NUMBER / ( radius * radius );
        for( size_t i = 0; i < count; ++i )
        {
            const float x = static_cast<float>( i ) / (  count - 1 ) - offset.x( );
            data[ i * channelCount + channel ] =
                    livre::maths::clamp( height * std::exp(- x * x * radiusFactor ) + offset.y( ),
                                         0.0f,
                                         1.0f );
        }
    }
    else
    {
        for( size_t i = 0; i < count; ++i )
        {
            data[ i * channelCount + channel ] =
                    livre::maths::clamp( ( ( i == start.x( ) * ( count - 1 ) ) ? height + offset.y( ) : offset.y( ) ),
                                         0.0f,
                                         1.0f );
        }
    }
}

AnchorGraphCore::AnchorVector GaussGraphCore::getCurve( const ColorChannel channel ) const
{
    AnchorGraphCore::AnchorVector anchors = anchorLists_[ channel ];
    Vector2f& start = anchors.front( );
    Vector2f& end = anchors.back( );
    start[ 1 ] =  1.0f - start[ 1 ];
    end[ 1 ] = 1.0f - end[ 1 ];
    return GaussGraphCore::transformAnchors( anchors );
}

void GaussGraphCore::setCurve( TransferFunction1Df& transferFunction,
                               const ColorChannel channel,
                               const AnchorVector &anchors )
{
    setAnchorVector( transferFunction, channel, transformParameters( anchors ) );
}






}
