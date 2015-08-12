
/* Copyright (c) 2006-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
 *                          Daniel.Nachbaur@epfl.ch
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

#include <livre/eq/settings/CameraSettings.h>

#include <livre/core/maths/maths.h>
#include <co/co.h>

namespace livre
{

CameraSettings::CameraSettings()
    : defaultCameraPosition_( Vector3f( 0.f, 0.f, 1.f ))
    , defaultCameraLookAt_( Vector3f::ZERO )
    , cameraRotation_( Matrix4f::IDENTITY )
    , modelRotation_( Matrix4f::IDENTITY )
    , cameraPosition_( defaultCameraPosition_ )
    , pilotMode_( false )
{
}

void CameraSettings::reset()
{
    pilotMode_ = false;
    cameraPosition_ = defaultCameraPosition_;
    cameraRotation_ = Matrix4f::IDENTITY;
    modelRotation_ = Matrix4f::IDENTITY;

    setCameraLookAt( defaultCameraLookAt_ );

    setDirty( DIRTY_ALL );
}

void CameraSettings::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );

    os << cameraRotation_
       << modelRotation_
       << cameraPosition_
       << pilotMode_;
}

void CameraSettings::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );

    is >> cameraRotation_
       >> modelRotation_
       >> cameraPosition_
       >> pilotMode_;
}

void CameraSettings::spinCamera( const float x, const float y )
{
    cameraRotation_.pre_rotate_x( x );
    cameraRotation_.pre_rotate_y( y );

    setDirty( DIRTY_ALL );
}

void CameraSettings::spinModel( const float x, const float y, const float z )
{
    if( x == 0.f && y == 0.f && z == 0.f )
        return;

    Matrix4f matInverse;
    cameraRotation_.inverse( matInverse );
    Vector4f shift = matInverse * Vector4f( x, y, z, 1 );
    modelRotation_.pre_rotate_x( shift.x( ));
    modelRotation_.pre_rotate_y( shift.y( ));
    modelRotation_.pre_rotate_z( shift.z( ));

    setDirty( DIRTY_ALL );
}

void CameraSettings::moveCamera( const float x, const float y, const float z )
{
    Vector3f oldPos = cameraPosition_;

    Matrix4f matInverse;
    cameraRotation_.inverse( matInverse );
    Vector4f shift = matInverse * Vector4f( x, y, z, 1 );
    cameraPosition_ += shift;

    oldPos -= cameraPosition_;

    setDirty( DIRTY_ALL );
}

void CameraSettings::setDefaultCameraPosition( const Vector3f& position )
{
    defaultCameraPosition_ = position;
    setCameraPosition( position );
}

void CameraSettings::setDefaultCameraLookAt( const Vector3f& lookAt )
{
    defaultCameraLookAt_ = lookAt;
    setCameraLookAt( lookAt );
}

void CameraSettings::setCameraPosition( const Vector3f& position )
{
    cameraPosition_ = position;
    setDirty( DIRTY_ALL );
}

void CameraSettings::togglePilotMode()
{
    pilotMode_ = !pilotMode_;
    setDirty( DIRTY_ALL );
}

bool CameraSettings::getPilotMode( ) const
{
    return pilotMode_;
}

void CameraSettings::setCameraRotation( const Vector3f& rotation )
{
    cameraRotation_ = Matrix4f::IDENTITY;
    cameraRotation_.rotate_x( rotation.x() );
    cameraRotation_.rotate_y( rotation.y() );
    cameraRotation_.rotate_z( rotation.z() );
    setDirty( DIRTY_ALL );
}

void CameraSettings::setCameraLookAt( const Vector3f& lookAt )
{
    setModelViewMatrix( maths::computeModelViewMatrix( cameraPosition_, lookAt ));
}

void CameraSettings::setModelViewMatrix( const Matrix4f& modelViewMatrix )
{
    Matrix3f rotationMatrix;
    maths::getRotationAndEyePositionFromModelView( modelViewMatrix,
                                                   rotationMatrix,
                                                   cameraPosition_ );
    if( pilotMode_ )
    {
        cameraPosition_ = -cameraPosition_;
        Matrix3f transpose = Matrix4f::IDENTITY;
        rotationMatrix.transpose_to( transpose );
        cameraRotation_.set_sub_matrix( transpose, 0, 0 );
        modelRotation_ = Matrix4f::IDENTITY;
    }
    else
    {
        Matrix3f inverseRotation = Matrix4f::IDENTITY;
        rotationMatrix.transpose_to( inverseRotation );
        modelRotation_ = inverseRotation;
        modelRotation_( 3, 3 ) = 1;
        cameraPosition_ = inverseRotation * -cameraPosition_;
        cameraRotation_ = Matrix4f::IDENTITY;
    }

    setDirty( DIRTY_ALL );
}

Matrix4f CameraSettings::getModelViewMatrix() const
{
    Matrix4f modelView;
    modelView = modelRotation_;
    modelView.set_translation( cameraPosition_ );
    modelView = cameraRotation_ * modelView;

    return modelView;
}

}
