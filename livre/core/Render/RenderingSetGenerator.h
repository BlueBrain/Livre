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

#ifndef _RenderingSetGenerator_h_
#define _RenderingSetGenerator_h_

#include <livre/core/types.h>
#include <livre/core/dashTypes.h>

#include <livre/core/Data/VolumeInformation.h>

namespace livre
{

/**
 * The RenderingSetGenerator class is used to generate the list of dash nodes to be rendered.
 */
class RenderingSetGenerator
{
public:
    /**
     * @param tree This parameter set the Dash Tree of Hierarchical Volume Data ( HVD ).
     */
    explicit RenderingSetGenerator( DashTreePtr tree );

    virtual ~RenderingSetGenerator();

    /**
     * Generates the rendering set according to the given frustum.
     * @param viewFrustum parameter is frustum to query HVD
     * @param allNodesList The list of nodes to be should have been rendered.
     * @param renderNodeList The list of nodes to be rendered.
     * @param notAvailableRenderNodeList The nodes which are not available for the frustum query.
     * @param renderBrickList The list of bricks to be rendered.
     */
    virtual void generateRenderingSet( const Frustum& viewFrustum,
                                       DashNodeVector& allNodesList,
                                       DashNodeVector& renderNodeList,
                                       DashNodeVector& notAvailableRenderNodeList,
                                       RenderBricks& renderBrickList ) = 0;

    /**
     * @return Get dash tree.
     */
    DashTreePtr getDashTree();

private:
    DashTreePtr _tree;
};

}

#endif // _RenderingSet_h_
