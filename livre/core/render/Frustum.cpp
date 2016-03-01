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
    const vmml::Visibility vis = _culler.test( worldBox );
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

const Vector3f& Frustum::getEyeCoords() const
{
    return eye_;
}

bool Frustum::operator==( const Frustum& rhs ) const
{
    return modelViewMatrix_.equals( rhs.getModelViewMatrix(),
                                    std::numeric_limits< float >::epsilon( ));
}


void Frustum::setup( const Matrix4f& modelViewMatrix,
                     const Matrix4f& projectionMatrix )
{
    const Matrix4f& mvp = projectionMatrix * modelViewMatrix;
    modelViewMatrix_ = modelViewMatrix;
    projectionMatrix_ = projectionMatrix;
    projectionMatrix_.inverse( invProjectionMatrix_ );

    _culler = FrustumCullerf( mvp );
    computeLimitsFromProjectionMatrix_();

    modelViewMatrix_.inverse( invModelViewMatrix_ );
    invModelViewMatrix_.get_translation( eye_ );
}

void Frustum::computeLimitsFromProjectionMatrix_()
{
    *static_cast< Frustumf* >( this ) = Frustumf( projectionMatrix_ );
}

}
