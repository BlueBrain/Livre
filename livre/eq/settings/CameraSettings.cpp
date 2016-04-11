
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
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
    : cameraPosition_( 0.f, 0.f, 1.5f )
{
}

void CameraSettings::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );
    os << modelRotation_ << cameraPosition_;
}

void CameraSettings::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );
    is >> modelRotation_ >> cameraPosition_;
}

void CameraSettings::spinModel( const float x, const float y, const float z )
{
    if( x == 0.f && y == 0.f && z == 0.f )
        return;

    modelRotation_.pre_rotate_x( x );
    modelRotation_.pre_rotate_y( y );
    modelRotation_.pre_rotate_z( z );
    setDirty( DIRTY_ALL );
}

void CameraSettings::moveCamera( const float x, const float y, const float z )
{
    cameraPosition_ += Vector3f( x, y, z );
    setDirty( DIRTY_ALL );
}

void CameraSettings::setCameraPosition( const Vector3f& position )
{
    cameraPosition_ = position;
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
    Matrix3f inverseRotation = Matrix4f::IDENTITY;
    rotationMatrix.transpose_to( inverseRotation );
    modelRotation_ = inverseRotation;
    modelRotation_( 3, 3 ) = 1;
    cameraPosition_ = inverseRotation * -cameraPosition_;

    setDirty( DIRTY_ALL );
}

Matrix4f CameraSettings::getModelViewMatrix() const
{
    Matrix4f modelView;
    modelView = modelRotation_;
    modelView.set_translation( cameraPosition_ );
    return modelView;
}

}
