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

struct Frustum::Impl
{
    Impl( Frustum& frustum, const Matrix4f& modelViewMatrix,
          const Matrix4f& projectionMatrix )
        : _frustum( frustum )
        , _mvMatrix( modelViewMatrix )
        , _invMVMatrix( _mvMatrix.inverse( ))
        , _projMatrix( projectionMatrix )
        , _invProjMatrix( _projMatrix.inverse( ))
        , _culler( projectionMatrix * modelViewMatrix )
    {
        computeLimitsFromProjectionMatrix();
        _eye = _invMVMatrix.getTranslation();

        const Vector4f eyeDir = _invMVMatrix.getColumn( 2 );
        _dir[ 0 ] = eyeDir[ 0 ];
        _dir[ 1 ] = eyeDir[ 1 ];
        _dir[ 2 ] = eyeDir[ 2 ];
    }

    const Plane& getNearPlane() const
    {
        return _culler.getNearPlane();
    }

    bool boxInFrustum( const Boxf &worldBox ) const
    {
        const vmml::Visibility vis = _culler.test( worldBox );
        return vis != vmml::VISIBILITY_NONE;
    }

    Matrix4f getMVPMatrix() const
    {
        return _projMatrix * _mvMatrix;
    }

    bool operator==( const Frustum::Impl& rhs ) const
    {
        return _mvMatrix.equals( rhs._mvMatrix,
                                 std::numeric_limits< float >::epsilon( ));
    }

    void computeLimitsFromProjectionMatrix()
    {
        *static_cast< Frustumf* >( &_frustum ) = Frustumf( _projMatrix );
    }

    Frustum& _frustum;
    const Matrix4f _mvMatrix;
    Matrix4f _invMVMatrix;
    const Matrix4f _projMatrix;
    Matrix4f _invProjMatrix;
    Vector3f _eye;
    Vector3f _dir;
    FrustumCullerf _culler;
};

Frustum::Frustum( const Matrix4f& modelViewMatrix,
                  const Matrix4f& projectionMatrix )
    : _impl( new Frustum::Impl( *this, modelViewMatrix, projectionMatrix ))
{}

Frustum::~Frustum()
{}

const Plane& Frustum::getNearPlane() const
{
    return _impl->getNearPlane();
}

bool Frustum::boxInFrustum( const Boxf& worldBox ) const
{
    return _impl->boxInFrustum( worldBox );
}

vmml::Visibility Frustum::getBoxIntersection( const Boxf& worldBox ) const
{
    return _impl->_culler.test( worldBox );
}

const Matrix4f& Frustum::getMVMatrix() const
{
    return _impl->_mvMatrix;
}

const Matrix4f& Frustum::getProjMatrix() const
{
    return _impl->_projMatrix;
}

const Matrix4f& Frustum::getInvMVMatrix() const
{
    return _impl->_invMVMatrix;
}

const Matrix4f& Frustum::getInvProjMatrix() const
{
    return _impl->_invProjMatrix;
}

Matrix4f Frustum::getMVPMatrix() const
{
    return _impl->getMVPMatrix();
}

const Vector3f& Frustum::getEyePos() const
{
    return _impl->_eye;
}

const Vector3f& Frustum::getViewDir() const
{
    return _impl->_dir;
}

bool Frustum::operator==( const Frustum& rhs ) const
{
    return *_impl == *rhs._impl;
}

}
