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

#include <livre/core/Maths/Maths.h>

namespace livre
{

namespace maths
{

void matrixToEulerAngles( const Matrix3f& rotationMatrix, float& roll, float& pitch, float& yaw )
{
    // http://www.flipcode.com/documents/matrfaq.html#Q37
    pitch = -std::asin( rotationMatrix[ 6 ] );        /* Calculate Y-axis angle */
    const float C =  std::cos( pitch );

    if ( std::abs( C ) > 0.005f )       /* Gimball lock? */
    {
        float trX = rotationMatrix[ 8 ] / C;  /* No, so get X-axis angle */
        float trY = -rotationMatrix[ 7 ]  / C;

        roll  = std::atan2( trY, trX );
        trX =  rotationMatrix[ 0 ] / C; /* Get Z-axis angle */
        trY = -rotationMatrix[ 3 ] / C;
        yaw  = std::atan2( trY, trX );
    }
    else                                 /* Gimball lock has occurred */
    {
        roll = 0.0;                      /* Set X-axis angle to zero */
        const float trX = rotationMatrix[ 4 ];                 /* And calculate Z-axis angle */
        const float trY = rotationMatrix[ 1 ];
        yaw  = std::atan2( trY, trX );
    }
}

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

    const Vector3f xAxis = vmml::normalize( up.cross( zAxis ));
    const Vector3f yAxis = zAxis.cross( xAxis );

    Matrix3f rotationMatrix = Matrix4f::IDENTITY;
    rotationMatrix.set_column( 0, xAxis );
    rotationMatrix.set_column( 1, yAxis );
    rotationMatrix.set_column( 2, zAxis );

    return computeModelViewMatrix( rotationMatrix, eye );
}

Matrix4f computeModelViewMatrix( const Quaternionf& quat, const Vector3f& eye )
{
    Matrix3f rotationMatrix;
    quat.get_rotation_matrix( rotationMatrix );
    return computeModelViewMatrix( rotationMatrix, eye );
}

Vector3f computePlaneIntersection( const Plane& plane0, const Plane& plane1, const Plane& plane2 )
{
    // TODO: Paralllel planes, whew !

    Matrix3f linSolve;
    linSolve.set_row( 0, plane0.getNormal() );
    linSolve.set_row( 1, plane1.getNormal() );
    linSolve.set_row( 2, plane2.getNormal() );

    Matrix3f ilinSolve;
    linSolve.inverse( ilinSolve );

    Vector3f bSolve( -plane0.getd(),
                     -plane1.getd(),
                     -plane2.getd() );

    return ilinSolve * bSolve;
}

Vector3f computePointOnLine( const Vector3f& p0, const Vector3f& p1, float t )
{
    const Vector3f diff = p1 - p0;
    const Vector3f add = diff * t;
    const Vector3f ret = p0 + add;
    return ret;
}

}
}
