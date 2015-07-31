/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _AvailableSetGenerator_h_
#define _AvailableSetGenerator_h_

#include <livre/core/render/RenderingSetGenerator.h>
#include <livre/lib/api.h>

namespace livre
{

/**
 * The AvailableSetGenerator class generates a rendering set according to the availability
 * of the textures.
 */
class AvailableSetGenerator : public RenderingSetGenerator
{
public:
    /**
     * @param tree The initialized dash tree with the volume.
     * @param windowHeight View height in pixels.
     * @param screenSpaceError Screen space error in pixels.
     */
    LIVRE_API AvailableSetGenerator( DashTreePtr tree);

    /**
     * Generates the rendering set according to the given frustum.
     * @param viewFrustum Frustum to query HVD
     * @param frameInfo Keeps the frame information
     */
    LIVRE_API void generateRenderingSet( const Frustum& viewFrustum,
                                         FrameInfo& frameInfo );
};


}
#endif // _AvailableSetGenerator_h_
