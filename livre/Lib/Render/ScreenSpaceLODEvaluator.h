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

#ifndef _ScreenSpaceLODEvaluator_h_
#define _ScreenSpaceLODEvaluator_h_

#include <livre/core/Render/LODEvaluator.h>

namespace livre
{

class ScreenSpaceLODEvaluator : public LODEvaluator
{
public:

    ScreenSpaceLODEvaluator( const uint32_t windowHeight,
                             const float screenSpaceError,
                             const float worldSpacePerVoxel,
                             const uint32_t minDepth,
                             const uint32_t maxDepth );

    uint32_t getLODForPoint( const Frustum& frustum,
                             const uint32_t volumeDepth,
                             const Vector3f& worldCoord ) const final;

private:

    const uint32_t _windowHeight;
    const float _screenSpaceError;
    const float _worldSpacePerVoxel;
    const uint32_t _minDepth;
    const uint32_t _maxDepth;
};

}

#endif // _ScreenSpaceLODSelectionAlgorithm_h_

