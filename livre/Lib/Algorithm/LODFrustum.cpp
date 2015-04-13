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

#include "LODFrustum.h"

#include <livre/core/Maths/Maths.h>
#include <livre/core/Render/gl.h>

namespace livre
{

LODFrustum::LODFrustum()
    : screenSpaceError_( 1.0 ),
      screenHeight_( 0.0 ),
      worldSpacePerVoxel_( 0.0 ),
      nodeWorldSize_( 0.0 ),
      nbLODLevels_( 0 ),
      totalNumberOfNodes_( 0 )
{
}

LODFrustum::LODFrustum( const Frustum &frustum,
                        const float screenSpaceError,
                        const uint32_t screenHeight,
                        const float worldSpacePerVoxel,
                        const float nodeWorldSize,
                        const uint32_t nbLODLevels,
                        const FloatVector &expandDistances )
    : frustum_( frustum ),
      screenSpaceError_( screenSpaceError ),
      screenHeight_( screenHeight ),
      worldSpacePerVoxel_( worldSpacePerVoxel ),
      nodeWorldSize_( nodeWorldSize ),
      nbLODLevels_( nbLODLevels ),
      totalNumberOfNodes_( 0 ),
      expandDistances_( expandDistances )
{
    frustumLODLowerLimits_.resize( nbLODLevels_ );
    frustumLODUpperLimits_.resize( nbLODLevels_ );

    baseFrustumLODLowerLimits_.resize( nbLODLevels_ );
    baseFrustumLODUpperLimits_.resize( nbLODLevels_ );

    frustumMaxNbLODNodes_.resize( nbLODLevels_, 0 );

    initializeWorldPlanes_( );
    computeCorners_( );
    computeFrustumLODLimits_( );
}

const Frustum& LODFrustum::getFrustum( ) const
{
    return frustum_;
}

uint32_t LODFrustum::getMaxNumberOfNodes( ) const
{
    return totalNumberOfNodes_;
}

const FloatVector& LODFrustum::getExpandDistances(  ) const
{
    return expandDistances_;
}

float LODFrustum::getExpandDistance( const PlaneId planeId ) const
{
    return expandDistances_[ (uint32_t)planeId ];
}

void LODFrustum::initializeWorldPlanes_( )
{
    for( uint32_t i = 0; i < 6; ++i )
    {
        mvPlanes_[ i ] = frustum_.getMVPlane( (PlaneId)i ).translate( expandDistances_[ i ] );
    }
}

Vector3f LODFrustum::computeCorner_( const PlaneId planeId0,
                                     const PlaneId planeId1,
                                     const PlaneId planeId2 ) const
{
    return maths::computePlaneIntersection( mvPlanes_[ (uint32_t)planeId0 ],
                                            mvPlanes_[ (uint32_t)planeId1 ],
                                            mvPlanes_[ (uint32_t)planeId2 ] );
}

// The computation below is expensive, should be optimized ( less linear algebra please )
void LODFrustum::computeCorners_( )
{
    nTopRight_ = computeCorner_( PL_NEAR, PL_TOP, PL_RIGHT );
    nBottomLeft_ = computeCorner_( PL_NEAR, PL_BOTTOM, PL_LEFT );

    fTopRight_ = computeCorner_( PL_FAR, PL_TOP, PL_RIGHT );
    fBottomLeft_ = computeCorner_( PL_FAR, PL_BOTTOM, PL_LEFT );

    nTopLeft_ = computeCorner_( PL_NEAR, PL_TOP, PL_LEFT );
    nBottomRight_ = computeCorner_( PL_NEAR, PL_BOTTOM, PL_RIGHT );

    fTopLeft_ = computeCorner_( PL_FAR, PL_TOP, PL_LEFT );
    fBottomRight_ = computeCorner_( PL_FAR, PL_BOTTOM, PL_RIGHT );
}

bool LODFrustum::boxInSubFrustum( const Boxf &worldBox, const int32_t lodLevel ) const
{
    const float n = frustum_.getFrustumLimits( PL_NEAR );
    const float f = frustum_.getFrustumLimits( PL_FAR );

    const float dn = expandDistances_[ PL_NEAR ];

    const float lowerLimit = frustumLODLowerLimits_[ lodLevel ];
    const float upperLimit = frustumLODUpperLimits_[ lodLevel ];

    if( ( upperLimit - lowerLimit ) <= std::numeric_limits< float >::epsilon()  )
        return false;

    // Check center is in
    float nDisplacememt =  dn - lowerLimit;
    const Plane& nPlane = frustum_.getWPlane( PL_NEAR ).translate( nDisplacememt );
    if( !nPlane.intersectOrUnder( worldBox ) )
        return false;

    float fDisplacememt =  upperLimit - dn + n - f;
    const Plane& fPlane = frustum_.getWPlane( PL_FAR ).translate( fDisplacememt );
    if( !fPlane.intersectOrUnder( worldBox ) )
        return false;

    for (uint32_t i = 0; i < 4; i++)
    {
        float displacement = expandDistances_[ (PlaneId)i ];
        const Plane& plane = frustum_.getWPlane( (PlaneId)i ).translate( displacement );

        if( !plane.intersectOrUnder( worldBox ) )
            return false;
    }
    return true;
}

bool LODFrustum::boxInFrustum( const Boxf& worldBox ) const
{
    for( uint32_t i = 0; i < 6; ++i )
    {
        const Plane& plane = frustum_.getWPlane( (PlaneId)i ).translate( expandDistances_[ i ] );
        if( !plane.intersectOrUnder( worldBox ) )
            return false;
    }
    return true;
}

void LODFrustum::computeBaseFrustumLODLimits_( )
{
    const float n = frustum_.getFrustumLimits( PL_NEAR );
    const float t = frustum_.getFrustumLimits( PL_TOP );
    const float b = frustum_.getFrustumLimits( PL_BOTTOM );

    const float lodFactor = worldSpacePerVoxel_ * ( float )screenHeight_ * n /
                            ( screenSpaceError_ * ( t - b ) );

    baseFrustumLODLowerLimits_[ 0 ] = n;
    for( uint32_t i = 0; i < nbLODLevels_; ++i )
    {
        baseFrustumLODUpperLimits_[ i ] =
                std::min( baseFrustumLODLowerLimits_[ i ] + (float)( 1 << i ) * lodFactor,
                          frustum_.getFrustumLimits( PL_FAR ) );

        if( i != ( nbLODLevels_ - 1 ) )
            baseFrustumLODLowerLimits_[ i + 1 ] = baseFrustumLODUpperLimits_[ i ];
        else
            baseFrustumLODUpperLimits_[ i ] = frustum_.getFrustumLimits( PL_FAR );
    }
}

void LODFrustum::computeFrustumLODLimits_( )
{
    computeBaseFrustumLODLimits_( );

    const float df = expandDistances_[ PL_FAR ];
    const float dn = expandDistances_[ PL_NEAR ];

    const float n = frustum_.getFrustumLimits( PL_NEAR );

    for( uint32_t i = 0; i < nbLODLevels_; ++i )
    {
        frustumLODLowerLimits_[ i ] = baseFrustumLODLowerLimits_[ i ] - n;
        frustumLODUpperLimits_[ i ] = baseFrustumLODUpperLimits_[ i ] - n + dn + df;
    }

    const float maxDist = df + dn + frustum_.getFrustumLimits( PL_FAR ) - n;

    const float rMaxDist = 1.0 / maxDist;

    FloatVector frustumLODUpperAreas;
    FloatVector frustumLODLowerAreas;

    frustumLODLowerAreas.resize( nbLODLevels_ );
    frustumLODUpperAreas.resize( nbLODLevels_ );

    // Computes areas
    for( uint32_t i = 0; i < nbLODLevels_; ++i )
    {
        const float currentWorldSize = nodeWorldSize_ * (float)( 1 << i );
        const float rCWS = 1.0f / currentWorldSize;
        float t = frustumLODLowerLimits_[ i ] * rMaxDist;
        const Vector3f& corner1 = maths::computePointOnLine( nTopRight_, fTopRight_, t );
        const Vector3f& corner2 = maths::computePointOnLine( nBottomLeft_, fBottomLeft_, t );
        const Vector3f& diff = corner2 - corner1;
        frustumLODLowerAreas[ i ] =
                std::ceil( std::abs( diff[ 0 ] * rCWS ) + 1.0f  ) * std::ceil( std::abs( diff[ 1 ] * rCWS ) + 1.0f );
    }

    for( uint32_t i = 0; i < nbLODLevels_; ++i )
    {
        const float currentWorldSize = nodeWorldSize_ * (float)( 1 << i );
        const float rCWS = 1.0f / currentWorldSize;
        float t = frustumLODUpperLimits_[ i ] * rMaxDist;
        const Vector3f& corner1 = maths::computePointOnLine( nTopRight_, fTopRight_, t );
        const Vector3f& corner2 = maths::computePointOnLine( nBottomLeft_, fBottomLeft_, t );
        const Vector3f& diff = corner2 - corner1;
        frustumLODUpperAreas[ i ] =
                std::ceil( std::abs( diff[ 0 ] * rCWS ) + 1.0f  ) * std::ceil( std::abs( diff[ 1 ] * rCWS ) + 1.0f );
    }

    // Compute number of nodes
    for( uint32_t i = 0; i < nbLODLevels_; ++i )
    {
        const float diff = frustumLODUpperLimits_[ i ] - frustumLODLowerLimits_[ i ];
        // if( diff <= std::numeric_limits< float >::epsilon() )
        //    continue;
        const float currentWorldSize = nodeWorldSize_ * (float)( 1 << i );
        const float rCWS = 1.0f / currentWorldSize;
        const float A = (  frustumLODLowerAreas[ i ] + frustumLODUpperAreas[ i ] ) * 0.5;

        if( i != nbLODLevels_ - 1 )
        {
            frustumMaxNbLODNodes_[ i ] =  std::ceil( A ) * std::ceil( diff * rCWS );
            totalNumberOfNodes_ +=  frustumMaxNbLODNodes_[ i ];
        }
    }
    frustumMaxNbLODNodes_[ nbLODLevels_ - 1 ] = frustumMaxNbLODNodes_[ nbLODLevels_ - 2 ] * 0.5;
    totalNumberOfNodes_ += frustumMaxNbLODNodes_[ nbLODLevels_ - 1 ];
}

}
