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

#include <livre/core/types.h>
#include <livre/core/Maths/Plane.h>

namespace livre
{

namespace maths
{

/**
 * Computes the euler angles in radians between 0 and 2pi.
 * @param rotationMatrix is the 3x3 rotation matrix.
 * @param roll return roll angle in radians.
 * @param pitch return pitch angle in radians.
 * @param yaw return yaw angle in radians.
 */
void matrixToEulerAngles( const Matrix3f& rotationMatrix, float& roll, float& pitch, float& yaw );

/**
 * Extracts rotation and eye position from model view matrix.
 * @param modelviewMatrix is the 4x4 modelview matrix.
 * @param rotationMatrix returns 3x3 rotation matrix.
 * @param eye returns 3 element eye vector in world space.
 */
void getRotationAndEyePositionFromModelView( const Matrix4f& modelviewMatrix,
                                                   Matrix3f& rotationMatrix,
                                                   Vector3f& eye );
/**
 * Computes model view matrix from rotation matrix and eye vector in world space.
 * @param rotationMatrix 3x3 rotation matrix.
 * @param eye vector in world space.
 * @return return 4x4 model view matrix.
 */
Matrix4f computeModelViewMatrix( const Matrix3f& rotationMatrix, const Vector3f& eye );

/**
 * Computes model view matrix from rotation matrix and eye vector in world space.
 * @param rotationMatrix 4x4 homogenus matrix.
 * @param eye vector in world space.
 * @return return 4x4 model view matrix.
 */
Matrix4f computeModelViewMatrix( const Matrix4f& rotationMatrix, const Vector3f& eye );

/**
 * computeModelViewMatrix computes the model view matrix from a quaternion.
 * @param quat is the quaternion.
 * @param eye vector in world space.
 * @return The model view matrix.
 */
Matrix4f computeModelViewMatrix( const Quaternionf& quat, const Vector3f& eye );

/**
 * Computes intersection between three planes.
 * @param plane0 is the first plane.
 * @param plane1 is the second plane.
 * @param plane2 is the third plane.
 * @return The intersection point.
 * @warning Parallel planes are not checked.
 */
Vector3f computePlaneIntersection( const Plane& plane0, const Plane& plane1, const Plane& plane2 );

/**
 * Computes a point bewteen two points, on the parametric value t [ 0..1 ]
 * @param p0 begining point.
 * @param p1 end point.
 * @param t parametric value [ 0..1 ]
 * @return The point.
 */
Vector3f computePointOnLine( const Vector3f& p0, const Vector3f& p1, float t );

/**
 * Clamps a number between limits.
 * @param number Number to clamp.
 * @param lower Lower limit.
 * @param upper Upper limit.
 * @return The clamped number.
 */
template< class T >
T clamp( const T number, const T lower, const T upper )
{
    return number < lower ? lower : ( number > upper ? upper : number );
}

}

}
