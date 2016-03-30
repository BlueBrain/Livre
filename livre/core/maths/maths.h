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

#include <livre/core/api.h>
#include <livre/core/mathTypes.h>
#include <livre/core/types.h>

namespace livre
{

namespace maths
{

/**
 * Extracts rotation and eye position from model view matrix.
 * @param modelViewMatrix is the 4x4 modelview matrix.
 * @param rotationMatrix returns 3x3 rotation matrix.
 * @param eye returns 3 element eye vector in world space.
 */
LIVRECORE_API void getRotationAndEyePositionFromModelView( const Matrix4f& modelViewMatrix,
                                                           Matrix3f& rotationMatrix,
                                                           Vector3f& eye );
/**
 * Computes model view matrix from rotation matrix and eye vector in world space.
 * @param rotationMatrix 3x3 rotation matrix.
 * @param eye vector in world space.
 * @return return 4x4 model view matrix.
 */
LIVRECORE_API Matrix4f computeModelViewMatrix( const Matrix3f& rotationMatrix, const Vector3f& eye );

/**
 * Computes model view matrix from eye vector and lookAt position in world space.
 * @param eye vector in world space.
 * @param center orientation center as lookAt vector in world space.
 * @return return 4x4 model view matrix.
 */
LIVRECORE_API Matrix4f computeModelViewMatrix( const Vector3f& eye, const Vector3f& center );

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


/**
 * Finds the level of detail for a given world position and screen space error
 * @param frustum is the view frustum
 * @param worldCoord is the world position
 * @param worldSpacePerVoxel is world space per voxel
 * @param volumeDepth is the depth of the volume
 * @param windowHeight is height of the window in pixels
 * @param screenSpaceError is the pixels per voxel
 * @param minLOD lod is clamped to minLOD and maxLOD
 * @param maxLOD lod is clamped to minLOD and maxLOD
 */
LIVRECORE_API uint32_t getLODForPoint( const Frustum& frustum,
                                       const Vector3f& worldCoord,
                                       float worldSpacePerVoxel,
                                       uint32_t volumeDepth,
                                       uint32_t windowHeight,
                                       float screenSpaceError,
                                       uint32_t minLOD,
                                       uint32_t maxLOD );

}

}
