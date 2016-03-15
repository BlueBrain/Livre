
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

#include <algorithm>

namespace livre
{

CameraSettings::CameraSettings()
{
    Matrix4f initial = Matrix4f::IDENTITY;
    std::copy( &initial.array[0], &initial.array[0] + 16, getMatrix( ));
}

void CameraSettings::spinModel( const float x, const float y )
{
    if( x == 0.f && y == 0.f )
        return;

    float matrixValues[16];
    std::copy( getMatrix(), getMatrix() + 16, matrixValues );

    Matrix4f modelview( &matrixValues[0], &matrixValues[0] + 16 );

    modelview[0][3] = 0.0f;
    modelview[1][3] = 0.0f;
    modelview[2][3] = 0.0f;

    modelview.pre_rotate_x( x );
    modelview.pre_rotate_y( y );

    modelview[0][3] = matrixValues[12];
    modelview[1][3] = matrixValues[13];
    modelview[2][3] = matrixValues[14];

    std::copy( &modelview.array[0], &modelview.array[0] + 16, getMatrix( ));
}

void CameraSettings::moveCamera( const float x, const float y, const float z )
{
    getMatrix()[12] += x;
    getMatrix()[13] += y;
    getMatrix()[14] += z;
}

void CameraSettings::setCameraPosition( const Vector3f& pos )
{
    getMatrix()[12] = pos.x();
    getMatrix()[13] = pos.y();
    getMatrix()[14] = pos.z();
}

void CameraSettings::setCameraLookAt( const Vector3f& lookAt )
{
    Vector3f eye( (float)getMatrix()[12],
                  (float)getMatrix()[13],
                  (float)getMatrix()[14]);
    const Vector3f zAxis = vmml::normalize( eye - lookAt );

    // Avoid Gimbal lock effect when looking upwards/downwards
    Vector3f up( Vector3f::UP );
    const float angle = zAxis.dot( up );
    if( 1.f - std::abs( angle ) < 0.0001f )
    {
        Vector3f right( Vector3f::RIGHT );
        if( angle > 0 ) // Looking downwards
            right = Vector3f::LEFT;
        up = up.rotate( 0.01f, right );
        up.normalize();
    }

    const Vector3f xAxis = vmml::normalize( vmml::cross( up, zAxis ));
    const Vector3f yAxis = vmml::cross( zAxis, xAxis );

    Matrix3f rotationMatrix = Matrix4f::IDENTITY;
    rotationMatrix.set_column( 0, xAxis );
    rotationMatrix.set_column( 1, yAxis );
    rotationMatrix.set_column( 2, zAxis );

    Matrix4f rotationTranspose = Matrix4f::IDENTITY;
    rotationTranspose.set_sub_matrix( rotationMatrix, 0, 0 );
    rotationTranspose = transpose( rotationTranspose );

    Matrix4f modelview = Matrix4f::IDENTITY;
    modelview.set_translation( -eye );
    modelview = rotationTranspose * modelview;

    std::copy( &modelview.array[0], &modelview.array[0] + 16, getMatrix( ));
}

Matrix4f CameraSettings::computeMatrix() const
{
    float matrixValues[16];
    std::copy( getMatrix(), getMatrix() + 16, matrixValues );

    Matrix4f modelview( &matrixValues[0], &matrixValues[0] + 16 );
    return modelview;
}
}
