
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 *               2013, Ahmet Bilgili        <ahmet.bilgili@epfl.ch>
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

#include <livre/core/maths/maths.h>
#include <livre/eq/types.h>
#include <livre/eq/settings/CameraSettings.h>

#ifdef LIVRE_USE_ZEQ
#  include <zeq/event.h>
#  include <zeq/publisher.h>
#  include <zeq/vocabulary.h>
#endif
#include <eq/eq.h>
#include <algorithm>

#ifndef M_PI_2
#  define M_PI_2 1.57079632679489661923
#endif

namespace livre
{

CameraSettings::CameraSettings()
    : defaultCameraPosition_( Vector3f( 0.f, 0.f, 2.f ))
    , defaultCameraLookAt_( Vector3f::ZERO )
    , cameraRotation_( Matrix4f::IDENTITY )
    , modelRotation_( Matrix4f::IDENTITY )
    , cameraPosition_( defaultCameraPosition_ )
    , cameraSpin_( 0.f )
    , cameraTranslation_( 0.f )
    , pilotMode_( false )
{
}

void CameraSettings::reset()
{
    pilotMode_ = false;
    cameraPosition_ = defaultCameraPosition_;
    cameraRotation_ = Matrix4f::IDENTITY;
    modelRotation_ = Matrix4f::IDENTITY;
    cameraSpin_ = 0.f;
    cameraTranslation_ = 0.f;

    setCameraLookAt( defaultCameraLookAt_ );

    setDirty( DIRTY_ALL );
}

void CameraSettings::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );

    os << cameraRotation_
       << modelRotation_
       << cameraPosition_
       << cameraSpin_
       << cameraTranslation_
       << pilotMode_;
}

void CameraSettings::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );

    is >> cameraRotation_
       >> modelRotation_
       >> cameraPosition_
       >> cameraSpin_
       >> cameraTranslation_
       >> pilotMode_;
}

void CameraSettings::spinCamera( const float x, const float y )
{
    cameraRotation_.pre_rotate_x( x );
    cameraRotation_.pre_rotate_y( y );

    cameraSpin_ = std::sqrt( x*x + y*y );

    setDirty( DIRTY_ALL );
}

void CameraSettings::spinModel( const float x, const float y, const float z )
{
    if( x == 0.f && y == 0.f && z == 0.f && cameraSpin_ == 0.f )
        return;

    Matrix4f matInverse;
    cameraRotation_.inverse( matInverse );
    Vector4f shift = matInverse * Vector4f( x, y, z, 1 );
    modelRotation_.pre_rotate_x( shift.x( ));
    modelRotation_.pre_rotate_y( shift.y( ));
    modelRotation_.pre_rotate_z( shift.z( ));

    cameraSpin_ = std::sqrt( x*x + y*y + z*z );

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

    cameraTranslation_ = oldPos.length();

    setDirty( DIRTY_ALL );
}

void CameraSettings::resetCameraSpin()
{
    cameraSpin_ = 0.f;
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

void CameraSettings::setPilotMode( bool pilotMode )
{
    pilotMode_ = pilotMode;
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

void CameraSettings::setModelRotation( const Vector3f& rotation )
{
    modelRotation_ = Matrix4f::IDENTITY;
    modelRotation_.rotate_x( rotation.x() );
    modelRotation_.rotate_y( rotation.y() );
    modelRotation_.rotate_z( rotation.z() );

    setDirty( DIRTY_ALL );
}

void livre::CameraSettings::setModelViewMatrix( const Matrix4f& modelViewMatrix )
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

Matrix4f livre::CameraSettings::getModelViewMatrix() const
{
    Matrix4f modelView;
    modelView = modelRotation_;
    modelView.set_translation( cameraPosition_ );
    modelView = cameraRotation_ * modelView;

    return modelView;
}

}
