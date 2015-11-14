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
     * Computes the LOD for a world space point.
     * @param worldCoord World position of a point.
     * @param volumeInfo Volume information.
     * @param viewport View port in pixels.
     * @param frustum The view furstum.
     * @param minLOD Minimum LOD to be rendered.
     * @param maxLOD Maximum LOD to be rendered.
     * @return Returns the LOD between min - max LOD.
     */
    virtual uint32_t getLODForPoint( const Vector3f& worldCoord,
                                     const VolumeInformation& volumeInfo,
                                     const PixelViewport& viewport,
                                     const Frustum& frustum,
                                     uint32_t minLOD,
                                     uint32_t maxLOD ) const = 0;

};

}

#endif // _LODEvaluator_h_

