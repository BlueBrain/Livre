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

#include <livre/Lib/Render/ScreenSpaceLODEvaluator.h>
#include <livre/core/Render/Frustum.h>
#include <livre/core/Maths/Maths.h>

namespace livre
{

ScreenSpaceLODEvaluator::ScreenSpaceLODEvaluator( const uint32_t windowHeight,
                                                  const float screenSpaceError,
                                                  const float worldSpacePerVoxel,
                                                  const uint32_t minLOD,
                                                  const uint32_t maxLOD )
    : LODEvaluator( )
    , _windowHeight( windowHeight )
    , _screenSpaceError( screenSpaceError )
    , _worldSpacePerVoxel( worldSpacePerVoxel )
    , _minLOD( minLOD )
    , _maxLOD( maxLOD )
{}

uint32_t ScreenSpaceLODEvaluator::getLODForPoint( const Frustum& frustum,
                                                  const uint32_t volumeDepth,
                                                  const Vector3f& worldCoord ) const
{
    const float t = frustum.getFrustumLimits( PL_TOP );
    const float b = frustum.getFrustumLimits( PL_BOTTOM );

    const float worldSpacePerPixel = (  t - b ) / _windowHeight;
    const float voxelPerPixel = _worldSpacePerVoxel  / worldSpacePerPixel * _screenSpaceError;

    const float distance = std::abs( frustum.getWPlane( PL_NEAR ).distance( worldCoord ));

    const float n = frustum.getFrustumLimits( PL_NEAR );
    const float voxelPerPixelInDistance = voxelPerPixel * distance / n;

    const uint32_t minLOD = std::min( _minLOD, volumeDepth );
    const uint32_t maxLOD = std::min( _maxLOD, volumeDepth );

    const uint32_t lod = std::min( std::max( std::log2( voxelPerPixelInDistance ), 0.0f ),
                                   (float)volumeDepth - 1 );

    return livre::maths::clamp( uint32_t( volumeDepth - lod - 1 ),
                                minLOD, maxLOD );

}

}

