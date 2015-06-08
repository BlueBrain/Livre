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

#include <livre/Lib/Visitor/LODSelectionVisitor.h>
#include <livre/Lib/Cache/TextureObject.h>
#include <livre/Lib/Cache/TextureDataObject.h>
#include <livre/Lib/Algorithm/LODFrustum.h>
#include <livre/core/Dash/DashTree.h>

namespace livre
{

LODSelectionVisitor::LODSelectionVisitor( DashTreePtr dashTree,
                                          const LODFrustum& lodFrustum )
    : RenderNodeVisitor( dashTree ),
      lodFrustum_( lodFrustum )
{
}

void LODSelectionVisitor::visit( DashRenderNode& renderNode, VisitState& state )
{
    const LODNode& lodNode = renderNode.getLODNode();

    if( !lodNode.isValid() )
        return;

    const Boxf& worldBox = lodNode.getWorldBox();

    const Plane& nearPlane = lodFrustum_.getFrustum().getWPlane( PL_NEAR );
    Vector3f vmin, vmax;
    nearPlane.getNearFarPoints( worldBox, vmin, vmax );

    const uint32_t lod = lodFrustum_.getLODForPoint( vmin );

    const bool isVisible = ( lod <= lodNode.getNodeId().getLevel( ));

    renderNode.setVisible( isVisible );

    state.setVisitChild( !isVisible );
}

}
