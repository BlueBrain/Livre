/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _LODSelectionVisitor_h_
#define _LODSelectionVisitor_h_

#include <livre/Lib/types.h>

#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Visitor/RenderNodeVisitor.h>

namespace livre
{

/**
 * The LODSelectionVisitor class computes the nodes to be loaded according to the given lod
 * frustum and request type. It marks the dash tree with the appropriate flags.
 */
class LODSelectionVisitor : public RenderNodeVisitor
{
public:

    /**
     * @param dashTree The data structure for dash tree.
     * @param lodFrustum Frustum to be queried.
     * @param requestType Type of query ( visible, texture data, data ).
     */
    LODSelectionVisitor( DashTreePtr dashTree,
                         const LODFrustum& lodFrustum,
                         const RequestType requestType );

    /**
     * Visits the node.
     * @param renderNode The node being visited.
     * @param state Visit state.
     */
    void visit( DashRenderNode& renderNode, VisitState& state ) final;

private:

    const LODFrustum& lodFrustum_;
    const RequestType requestType_;
};

}

#endif // _LODSelectionVisitor_h_
