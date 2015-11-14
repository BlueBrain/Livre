/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                  Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/lib/render/ScreenSpaceLODEvaluator.h>
#include <livre/core/data/VolumeInformation.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/maths/maths.h>

namespace livre
{

ScreenSpaceLODEvaluator::ScreenSpaceLODEvaluator( const float screenSpaceError )
    : LODEvaluator( )
    , _screenSpaceError( screenSpaceError )
{}

uint32_t ScreenSpaceLODEvaluator::getLODForPoint( const Vector3f& worldCoord,
                                                  const VolumeInformation& volumeInfo,
                                                  const PixelViewport& viewport,
                                                  const Frustum& frustum,
                                                  uint32_t minLOD,
                                                  uint32_t maxLOD ) const
{
    const float t = frustum.getFrustumLimits( PL_TOP );
    const float b = frustum.getFrustumLimits( PL_BOTTOM );
    const uint32_t windowHeight = viewport[ 3 ];

    const float worldSpacePerPixel = (  t - b ) / windowHeight;
    const float worldSpacePerVoxel = volumeInfo.worldSpacePerVoxel;
    const float voxelPerPixel = worldSpacePerPixel  / worldSpacePerVoxel * _screenSpaceError;

    const float distance = std::abs( frustum.getWPlane( PL_NEAR ).distance( worldCoord ));

    const float n = frustum.getFrustumLimits( PL_NEAR );
    const float voxelPerPixelInDistance = voxelPerPixel * distance / n;

    const uint32_t volumeDepth = volumeInfo.rootNode.getDepth();
    const uint32_t lodMin = std::min( minLOD, volumeDepth );
    const uint32_t lodMax = std::min( maxLOD, volumeDepth );

    const uint32_t lod = std::min( std::max( std::log2( voxelPerPixelInDistance ), 0.0f ),
                                   (float)volumeDepth - 1 );

    return livre::maths::clamp( volumeDepth - lod - 1, lodMin, lodMax );

}

}

