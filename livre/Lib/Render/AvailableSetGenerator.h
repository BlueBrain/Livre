/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                         Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/core/Render/RenderingSetGenerator.h>

namespace livre
{

/**
 * The AvailableSetGenerator class, generates a rendering set according to the availability
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
    AvailableSetGenerator( DashTreePtr tree,
                           const uint32_t windowHeight,
                           const float screenSpaceError );

    /**
     * Generates the rendering set according to the given frustum.
     * @param viewFrustum Frustum to query HVD
     * @param allNodesList The list of nodes expected to be rendered.
     * @param renderNodeList The list of nodes to be rendered.
     * @param notAvailableRenderNodeList The nodes which are not available for the frustum query.
     * @param renderBrickList The list of bricks to be rendered.
     */
    void generateRenderingSet( const Frustum& viewFrustum,
                               DashNodeVector& allNodesList,
                               DashNodeVector& renderNodeList,
                               DashNodeVector& notAvailableRenderNodeList,
                               RenderBricks& renderBrickList );

private:

    const uint32_t _windowHeight;
    const float _screenSpaceError;
};


}
#endif // _AvailableSetGenerator_h_
