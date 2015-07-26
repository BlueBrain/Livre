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

#include <livre/core/api.h>
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
    LIVRECORE_API explicit RenderingSetGenerator( DashTreePtr tree );

    LIVRECORE_API virtual ~RenderingSetGenerator();

    /**
     * Generates the rendering set according to the given frustum.
     * @param viewFrustum Frustum to query HVD
     * @param frameInfo keeps the frame information
     */
    virtual void generateRenderingSet( const Frustum& viewFrustum,
                                       FrameInfo& frameInfo ) = 0;

    /**
     * @return Get dash tree.
     */
    LIVRECORE_API DashTreePtr getDashTree();

private:
    DashTreePtr _tree;
};

}

#endif // _RenderingSet_h_
