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

#include <livre/core/maths/maths.h>
#include <vmmlib/matrix.hpp>

namespace livre
{
namespace maths
{

void getRotationAndEyePositionFromModelView( const Matrix4f& modelViewMatrix,
                                                   Matrix3f& rotationMatrix,
                                                   Vector3f& eye )
{
    Matrix4f iMv;
    modelViewMatrix.inverse( iMv );
    iMv.get_sub_matrix( rotationMatrix, 0, 0 );
    iMv.get_translation( eye );
}

Matrix4f computeModelViewMatrix( const Matrix3f& rotationMatrix, const Vector3f& eye )
{
    Matrix4f rotationTranspose = Matrix4f::IDENTITY;
    rotationTranspose.set_sub_matrix( rotationMatrix, 0, 0 );
    rotationTranspose = transpose( rotationTranspose );

    Matrix4f modelViewMatrix = Matrix4f::IDENTITY;
    modelViewMatrix.set_translation( -eye );
    return rotationTranspose * modelViewMatrix;
}

Matrix4f computeModelViewMatrix( const Vector3f& eye, const Vector3f& center )
{
    const Vector3f zAxis = vmml::normalize( eye - center );

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

    return computeModelViewMatrix( rotationMatrix, eye );
}

}
}
