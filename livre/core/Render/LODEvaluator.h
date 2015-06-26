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

#ifndef _LODEvaluator_h_
#define _LODEvaluator_h_

#include <livre/core/types.h>
#include <livre/core/mathTypes.h>

namespace livre
{

/* The base class for selecting a LOD for a given point in view space */

class LODEvaluator
{
public:
    LODEvaluator() {}
    virtual ~LODEvaluator() {}

    /**
     * Calcualtes a level of detail with given parameters and the frustum.
     * @param frustum view frustum.
     * @param volumeDepth Depth of the volume.
     * @param worldCoord The 3D world coordinate
     * @return
     */
    virtual uint32_t getLODForPoint(  const Frustum& frustum,
                                      const uint32_t volumeDepth,
                                      const Vector3f& worldCoord ) const = 0;

};

}

#endif // _LODEvaluator_h_

