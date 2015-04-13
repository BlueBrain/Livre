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

#ifndef _GRSLowerLOD_h_
#define _GRSLowerLOD_h_

#include <livre/core/Render/GenerateRenderingSet.h>

namespace livre
{

/**
 * The GRSLowerLOD class, generates a rendering set where if there is no lod node to be rendered, it computes
 * the closest parent to be rendered.
 */
class GRSLowerLOD : public GenerateRenderingSet
{
public:

    /**
     * @param volumeInformation The \see VolumeInformation
     * @param tree The initialized dash tree with the volume.
     * @param windowHeight View height in pixels.
     * @param screenSpaceError Screen space error.
     */
    GRSLowerLOD( const VolumeInformation& volumeInformation,
                 dash::NodePtr tree,
                 const uint32_t windowHeight,
                 const float screenSpaceError );

    /**
     * Generates the rendering set according to the given frustum.
     * @param viewFrustum parameter is frustum to query HVD
     * @param allNodesList The list of nodes to be should have been rendered.
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

    const uint32_t windowHeight_;
    const float screenSpaceError_;
};


}
#endif // _GVSLowerLOD_h_
