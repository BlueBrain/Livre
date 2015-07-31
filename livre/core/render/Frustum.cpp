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

#include <livre/core/render/Frustum.h>

namespace livre
{

Frustum::Frustum()
    : modelViewMatrix_( Matrix4f::IDENTITY )
    , isInitialized_( false )
    , fovy_( 0 )
    , fovx_( 0 )
{
}

const Plane& Frustum::getWPlane( const PlaneId id ) const
{
    return wPlanes_[ (uint32_t)id ];
}

const Plane& Frustum::getMVPlane( const PlaneId id ) const
{
    return mvPlanes_[ (uint32_t)id ];
}

bool Frustum::boxInFrustum( const Boxf &worldBox ) const
{
    const Vector3f& min = worldBox.getMin();
    const Vector3f& max = worldBox.getMax();
    const Vector2f x( min[0], max[0] );
    const Vector2f y( min[1], max[1] );
    const Vector2f z( min[2], max[2] );

    const vmml::Visibility vis = vmmlFrustumCuller_.test_aabb( x, y, z );
    return vis != vmml::VISIBILITY_NONE;
}

bool Frustum::isInitialized( ) const
{
    return isInitialized_;
}

void Frustum::reset( )
{
    isInitialized_ = false;
}

const Matrix4f& Frustum::getModelViewMatrix( ) const
{
    return modelViewMatrix_;
}

const Matrix4f& Frustum::getProjectionMatrix( ) const
{
    return projectionMatrix_;
}

const Matrix4f& Frustum::getInvModelViewMatrix( ) const
{
    return invModelViewMatrix_;
}

const Matrix4f& Frustum::getInvProjectionMatrix( ) const
{
    return invProjectionMatrix_;
}

const Matrix4f& Frustum::getModelViewProjectionMatrix( ) const
{
    return mvpMatrix_;
}

float Frustum::getFOV( ) const
{
    return fovy_;
}

float Frustum::getFrustumLimits( const PlaneId planeId ) const
{
    return vmmlFrustum_.array[ planeId ];
}

const Vector3f& Frustum::getEyeCoords( ) const
{
    return eye_;
}

const Vector3f& Frustum::getViewDir( ) const
{
    return viewDir_;
}

const Vector4f& Frustum::getPlaneCenter( const PlaneId planeId ) const
{
    return planeCenters_[ planeId ];
}

bool Frustum::operator==( const Frustum& frustum ) const
{
    const Matrix4f& mv = frustum.getModelViewMatrix();

    for( uint32_t i = 0; i < 15; ++i )
    {
        if( std::abs( modelViewMatrix_.array[ i ] - mv.array[ i ] ) > std::numeric_limits< float >::epsilon() )
            return false;
    }
    return true;
}


void Frustum::initialize( const Matrix4f& modelViewMatrix,
                          const Matrix4f& projectionMatrix )
{
    if( isInitialized_ )
        return;

    mvpMatrix_ =  projectionMatrix * modelViewMatrix;
    modelViewMatrix_ = modelViewMatrix;
    projectionMatrix_ = projectionMatrix;
    projectionMatrix_.inverse( invProjectionMatrix_ );

    vmmlFrustumCuller_.setup( mvpMatrix_ );
    computeLimitsFromProjectionMatrix_( );
    initializePlanes_( mvpMatrix_, wPlanes_ );
    initializePlanes_( projectionMatrix_, mvPlanes_ );
    initializePlaneCenters_( );

    modelViewMatrix_.inverse( invModelViewMatrix_ );
    invModelViewMatrix_.get_translation( eye_ );
    const Vector4f& column = invModelViewMatrix_.get_column( 2 );

    viewDir_[ 0 ] = column[ 0 ];
    viewDir_[ 1 ] = column[ 1 ];
    viewDir_[ 2 ] = column[ 2 ];

    // Meaningful for only symmetric frusta
    fovx_ = std::atan( std::abs( vmmlFrustum_.array[ PL_LEFT ] )
                        / std::abs( vmmlFrustum_.array[ PL_NEAR ] ) ) * 2.0f;
    fovy_ = std::atan( std::abs( vmmlFrustum_.array[ PL_TOP ] )
                        / std::abs( vmmlFrustum_.array[ PL_NEAR ] ) ) * 2.0f;
    isInitialized_ = true;
}

void Frustum::initialize( const Matrix4f &modelViewMatrix,
                          float l,
                          float r,
                          float b,
                          float t,
                          float n,
                          float f )
{
    if( isInitialized_ )
        return;

    modelViewMatrix_ = modelViewMatrix;
    modelViewMatrix_.inverse( invModelViewMatrix_ );

    vmmlFrustum_.set( l, r ,b, t, n, f );
    projectionMatrix_ = vmmlFrustum_.compute_matrix();
    projectionMatrix_.inverse( invProjectionMatrix_ );

    invModelViewMatrix_.get_translation( eye_ );
    viewDir_ = invModelViewMatrix_.get_column( 2 );
    mvpMatrix_ =  projectionMatrix_* modelViewMatrix;
    vmmlFrustumCuller_.setup( mvpMatrix_ );
    initializePlanes_( mvpMatrix_, wPlanes_ );
    initializePlanes_( projectionMatrix_, mvPlanes_ );
    initializePlaneCenters_( );

    // Meaningful for only symmetric frusta
    fovx_ = std::atan( std::abs( vmmlFrustum_.array[ PL_LEFT ] ) /
                       std::abs( vmmlFrustum_.array[ PL_NEAR ] ) ) * 2.0f;
    fovy_ = std::atan( std::abs( vmmlFrustum_.array[ PL_TOP ] ) /
                       std::abs( vmmlFrustum_.array[ PL_NEAR ] ) ) * 2.0f;
    isInitialized_ = true;
}


void Frustum::computeLimitsFromProjectionMatrix_( )
{
    const float n = projectionMatrix_[2][3]/(projectionMatrix_[2][2] - 1.0);
    const float f = projectionMatrix_[2][3]/(projectionMatrix_[2][2] + 1.0);
    const float b = n * ( projectionMatrix_[1][2] - 1.0 ) / projectionMatrix_[1][1];
    const float t = n * ( projectionMatrix_[1][2] + 1.0 ) / projectionMatrix_[1][1];
    const float l = n * ( projectionMatrix_[0][2] - 1.0 ) / projectionMatrix_[0][0];
    const float r = n * ( projectionMatrix_[0][2] + 1.0 ) / projectionMatrix_[0][0];
    vmmlFrustum_.set( l, r ,b, t, n, f);

}

void Frustum::initializePlanes_( const Matrix4f& matrix, Plane* planes )
{
    // We could have got from the frustum culler but those are private :(
    const Vector4f& row0 = matrix.get_row( 0 );
    const Vector4f& row1 = matrix.get_row( 1 );
    const Vector4f& row2 = matrix.get_row( 2 );
    const Vector4f& row3 = matrix.get_row( 3 );

    planes[ PL_LEFT ] = Plane( row3 + row0 );
    planes[ PL_RIGHT ] = Plane( row3 - row0 );
    planes[ PL_BOTTOM ] = Plane( row3 + row1 );
    planes[ PL_TOP ] = Plane( row3 - row1 );
    planes[ PL_NEAR ] = Plane( row3 + row2 );
    planes[ PL_FAR ] = Plane( row3 - row2 );
}

void Frustum::initializePlaneCenters_()
{
    Matrix4f invMvp_;
    mvpMatrix_.inverse( invMvp_ );

    planeCenters_[ PL_NEAR ] = invMvp_ * Vector4f( 0.0f, 0.0f, -0.5f, 1.0f );
    planeCenters_[ PL_FAR ] = invMvp_ * Vector4f( 0.0f, 0.0f, 0.5f, 1.0f );
    planeCenters_[ PL_LEFT ] = invMvp_ * Vector4f( -0.5f, 0.0f, 0.0f, 1.0f );
    planeCenters_[ PL_RIGHT ] = invMvp_ * Vector4f( 0.5f, 0.0f, 0.0f, 1.0f );
    planeCenters_[ PL_TOP ] = invMvp_ * Vector4f( 0.0f, 0.5f, 0.0f, 1.0f );
    planeCenters_[ PL_BOTTOM ] = invMvp_ * Vector4f( 0.0f, -0.5f, 0.0f, 1.0f );


    planeCenters_[ PL_NEAR ] = planeCenters_[ PL_NEAR ] / planeCenters_[ PL_NEAR ][ 3 ];
    planeCenters_[ PL_FAR ] = planeCenters_[ PL_FAR ] / planeCenters_[ PL_FAR ][ 3 ];
    planeCenters_[ PL_LEFT ] = planeCenters_[ PL_LEFT ] / planeCenters_[ PL_LEFT ][ 3 ];
    planeCenters_[ PL_RIGHT ] = planeCenters_[ PL_RIGHT ] / planeCenters_[ PL_RIGHT ][ 3 ];
    planeCenters_[ PL_TOP ] = planeCenters_[ PL_TOP ] / planeCenters_[ PL_TOP ][ 3 ];
    planeCenters_[ PL_BOTTOM ] = planeCenters_[ PL_BOTTOM ] / planeCenters_[ PL_BOTTOM ][ 3 ];
}

void Frustum::computeFrustumVertices_( Vector3f frustumVertices[],
                                       Vector3f frustumNormals[] ) const
{
    float farLeft;
    float farRight;
    float farBottom;
    float farTop;

    float f = vmmlFrustum_.array[ PL_FAR ];
    float l = vmmlFrustum_.array[ PL_LEFT ];
    float r = vmmlFrustum_.array[ PL_RIGHT ];
    float b = vmmlFrustum_.array[ PL_BOTTOM ];
    float t = vmmlFrustum_.array[ PL_TOP ];
    float n = vmmlFrustum_.array[ PL_NEAR ];

    const float ratio  = f / n;
    farLeft   = l * ratio;
    farRight  = r * ratio;
    farBottom = b * ratio;
    farTop    = t * ratio;

    // compute 8 vertices of the frustum
    // near top right
    frustumVertices[0][0] = r;
    frustumVertices[0][1] = t;
    frustumVertices[0][2] = -n;

    // near top left
    frustumVertices[1][0] = l;
    frustumVertices[1][1] = t;
    frustumVertices[1][2] = -n;

    // near bottom left
    frustumVertices[2][0] = l;
    frustumVertices[2][1] = b;
    frustumVertices[2][2] = -n;

    // near bottom right
    frustumVertices[3][0] = r;
    frustumVertices[3][1] = b;
    frustumVertices[3][2] = -n;

    // far top right
    frustumVertices[4][0] = farRight;
    frustumVertices[4][1] = farTop;
    frustumVertices[4][2] = -f;

    // far top left
    frustumVertices[5][0] = farLeft;
    frustumVertices[5][1] = farTop;
    frustumVertices[5][2] = -f;

    // far bottom left
    frustumVertices[6][0] = farLeft;
    frustumVertices[6][1] = farBottom;
    frustumVertices[6][2] = -f;

    // far bottom right
    frustumVertices[7][0] = farRight;
    frustumVertices[7][1] = farBottom;
    frustumVertices[7][2] = -f;

    // compute normals
    frustumNormals[0] = (frustumVertices[5]
            - frustumVertices[1]).cross(frustumVertices[2]
            - frustumVertices[1]);
    frustumNormals[0].normalize();

    frustumNormals[1] = (frustumVertices[3]
            - frustumVertices[0]).cross(frustumVertices[4]
            - frustumVertices[0]);
    frustumNormals[1].normalize();

    frustumNormals[2] = (frustumVertices[6]
            - frustumVertices[2]).cross(frustumVertices[3]
            - frustumVertices[2]);
    frustumNormals[2].normalize();

    frustumNormals[3] = (frustumVertices[4]
            - frustumVertices[0]).cross(frustumVertices[1]
            - frustumVertices[0]);
    frustumNormals[3].normalize();

    frustumNormals[4] = (frustumVertices[1]
            - frustumVertices[0]).cross(frustumVertices[3]
            - frustumVertices[0]);
    frustumNormals[4].normalize();

    frustumNormals[5] = (frustumVertices[7]
            - frustumVertices[4]).cross(frustumVertices[5]
            - frustumVertices[4]);
    frustumNormals[5].normalize();
}

}
