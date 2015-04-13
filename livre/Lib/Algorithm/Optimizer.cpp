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

#include <livre/core/Maths/BinarySearch.h>

#include <livre/Lib/Algorithm/Optimizer.h>
#include <livre/Lib/Algorithm/LODFrustum.h>
#include <livre/Lib/Configuration/EFPrefetchAlgorithmParameters.h>

#include <algorithm>

namespace livre
{

struct SumNodesForPrefetchDistanceFunction
{

    SumNodesForPrefetchDistanceFunction(  const Frustum &frustum,
                                          const float worldSpacePerVoxel,
                                          const float screenHeight,
                                          const float screenSpaceError,
                                          const uint32_t nbLODLevels,
                                          const float memoryPerLOD,
                                          const float nodeWorldSize,
                                          const FloatVector& planeExpansionRatios )
     : frustum_( frustum ),
       worldSpacePerVoxel_( worldSpacePerVoxel ),
       screenHeight_( screenHeight ),
       screenSpaceError_( screenSpaceError ),
       nbLODLevels_( nbLODLevels ),
       memoryPerLOD_( memoryPerLOD ),
       nodeWorldSize_( nodeWorldSize ),
       planeExpansionRatios_( planeExpansionRatios )
    {}


    float operator()( const float delta );

    const Frustum &frustum_;
    const float worldSpacePerVoxel_;
    const float screenHeight_;
    const float screenSpaceError_;
    const uint32_t nbLODLevels_;
    const float memoryPerLOD_;
    const float nodeWorldSize_;
    const FloatVector& planeExpansionRatios_;
    LODFrustum lastComputedLODFrustum_;
};

float SumNodesForPrefetchDistanceFunction::operator()( const float d )
{
    FloatVector distances;
    distances.resize( 6 );

    for( uint32_t i = 0; i < 6; ++i )
        distances[ i ] = d * planeExpansionRatios_[ i ];

    lastComputedLODFrustum_ = LODFrustum( frustum_,
                                          screenSpaceError_,
                                          screenHeight_,
                                          worldSpacePerVoxel_,
                                          nodeWorldSize_,
                                          nbLODLevels_,
                                          distances );

    return lastComputedLODFrustum_.getMaxNumberOfNodes();
}

LODFrustum computeLODFrustum( const Frustum &frustum,
                              const uint32_t maxMemory,
                              const float worldSpacePerVoxel,
                              const float screenHeight,
                              const float screenSpaceError,
                              const uint32_t nbLODLevels,
                              const float memoryPerLOD,
                              const float nodeWorldSize,
                              const float optimizerPrecision,
                              const FloatVector&planeExpansionRatios,
                              bool& error )
{

    SumNodesForPrefetchDistanceFunction searchFunction( frustum,
                                                        worldSpacePerVoxel,
                                                        screenHeight,
                                                        screenSpaceError,
                                                        nbLODLevels,
                                                        memoryPerLOD,
                                                        nodeWorldSize,
                                                        planeExpansionRatios );

    binarySearch( searchFunction,
                  0.0f,
                  0.5f,
                  std::floor( maxMemory / memoryPerLOD ),
                  optimizerPrecision,
                  error );

    return searchFunction.lastComputedLODFrustum_;


}




}
