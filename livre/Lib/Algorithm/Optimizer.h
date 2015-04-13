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

#ifndef _Optimizer_h_
#define _Optimizer_h_

#include <livre/Lib/types.h>
#include <livre/Lib/Algorithm/LODFrustum.h>

namespace livre
{

/**
 * Computes the corresponding LOD frustum based on quality and limitation params.
 * @param frustum The view-frustum.
 * @param maxMemory Maximum memory limitation.
 * @param worldSpacePerVoxel World space per pixel in the highest resolution ( 1.0 / max volume dim ).
 * @param screenHeight The height of rendering screen.
 * @param screenSpaceError Selects level of detail according to given screen space error.
 * @param nbLODLevels The depth of the Hierarchical Volume Data ( HVD ) tree.
 * @param memoryPerLOD The max memory size per node.
 * @param nodeWorldSize Node world size in the highest resolution.
 * @param optimizerPrecision
 * @param planeExpansionRatios If needed the planes of view-frustum can be moved by given distances.
 * @param error If no frustum can be found for given parameters error is true.
 * @return The computed \see LODFrustum.
 */
 LODFrustum computeLODFrustum( const Frustum &frustum,
                               const uint32_t maxMemory,
                               const float worldSpacePerVoxel,
                               const float screenHeight,
                               const float screenSpaceError,
                               const uint32_t nbLODLevels,
                               const float memoryPerLOD,
                               const float nodeWorldSize,
                               const float optimizerPrecision,
                               const FloatVector& planeExpansionRatios,
                               bool& error );

}
#endif // _Optimizer_h_
