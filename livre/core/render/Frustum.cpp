/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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
{}

const Plane& Frustum::getNearPlane() const
{
    return _culler.getNearPlane();
}

bool Frustum::boxInFrustum( const Boxf &worldBox ) const
{
    const Vector3f& min = worldBox.getMin();
    const Vector3f& max = worldBox.getMax();
    const Vector2f x( min[0], max[0] );
    const Vector2f y( min[1], max[1] );
    const Vector2f z( min[2], max[2] );

    const vmml::Visibility vis = _culler.test_aabb( x, y, z );
    return vis != vmml::VISIBILITY_NONE;
}

const Matrix4f& Frustum::getModelViewMatrix() const
{
    return modelViewMatrix_;
}

const Matrix4f& Frustum::getProjectionMatrix() const
{
    return projectionMatrix_;
}

const Matrix4f& Frustum::getInvModelViewMatrix() const
{
    return invModelViewMatrix_;
}

const Matrix4f& Frustum::getInvProjectionMatrix() const
{
    return invProjectionMatrix_;
}

Matrix4f Frustum::getModelViewProjectionMatrix() const
{
    return projectionMatrix_ * modelViewMatrix_;
}

float Frustum::getFrustumLimits( const PlaneId planeId ) const
{
    return vmmlFrustum_.array[ planeId ];
}

const Vector3f& Frustum::getEyeCoords() const
{
    return eye_;
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


void Frustum::setup( const Matrix4f& modelViewMatrix,
                     const Matrix4f& projectionMatrix )
{
    const Matrix4f& mvp = projectionMatrix * modelViewMatrix;
    modelViewMatrix_ = modelViewMatrix;
    projectionMatrix_ = projectionMatrix;
    projectionMatrix_.inverse( invProjectionMatrix_ );

    _culler.setup( mvp );
    computeLimitsFromProjectionMatrix_();

    modelViewMatrix_.inverse( invModelViewMatrix_ );
    invModelViewMatrix_.get_translation( eye_ );
}

void Frustum::computeLimitsFromProjectionMatrix_()
{
    const float n = projectionMatrix_[2][3]/(projectionMatrix_[2][2] - 1.0);
    const float f = projectionMatrix_[2][3]/(projectionMatrix_[2][2] + 1.0);
    const float b = n * ( projectionMatrix_[1][2] - 1.0 ) / projectionMatrix_[1][1];
    const float t = n * ( projectionMatrix_[1][2] + 1.0 ) / projectionMatrix_[1][1];
    const float l = n * ( projectionMatrix_[0][2] - 1.0 ) / projectionMatrix_[0][0];
    const float r = n * ( projectionMatrix_[0][2] + 1.0 ) / projectionMatrix_[0][0];
    vmmlFrustum_.set( l, r ,b, t, n, f);

}

}
