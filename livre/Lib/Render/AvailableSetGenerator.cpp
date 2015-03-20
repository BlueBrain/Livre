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

#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Dash/DashTree.h>
#include <livre/core/Render/RenderBrick.h>
#include <livre/core/Data/VolumeDataSource.h>
#include <livre/core/Data/VolumeInformation.h>
#include <livre/core/Visitor/RenderNodeVisitor.h>
#include <livre/Lib/Render/AvailableSetGenerator.h>
#include <livre/Lib/Visitor/CollectionTraversal.h>
#include <livre/Lib/Visitor/DFSTraversal.h>
#include <livre/Lib/Cache/TextureObject.h>
#include <livre/Lib/Algorithm/LODFrustum.h>

namespace livre
{

typedef boost::unordered_map< NodeId, dash::NodePtr > NodeIdDashNodeMap;

class VisibleCollectorVisitor : public RenderNodeVisitor
{
public:

    VisibleCollectorVisitor( DashTreePtr dashTree,
                             const Frustum& frustum,
                             const float screenSpaceError,
                             const uint32_t windowHeight,
                             DashNodeVector& nodeVector );

    void visit( DashRenderNode& renderNode, VisitState& state ) final;

    LODFrustum lodFrustum_;
    DashNodeVector& nodeVector_;
};

VisibleCollectorVisitor::VisibleCollectorVisitor(
        DashTreePtr dashTree,
        const Frustum& frustum,
        const float screenSpaceError,
        const uint32_t windowHeight,
        DashNodeVector& nodeVector )
    : RenderNodeVisitor( dashTree ),
      nodeVector_( nodeVector )
{
    FloatVector distances;

    const float frustumSurfaceDelta = 0.0f;
    distances.resize( PL_FAR + 1, frustumSurfaceDelta );

    const VolumeInformation& volumeInformation =
            dashTree->getDataSource()->getVolumeInformation();
    const float wsPerVoxel = volumeInformation.worldSpacePerVoxel;
    const float depth = volumeInformation.rootNode.getDepth();
    const float levelZeroNodeSize = float( volumeInformation.maximumBlockSize[ 0 ] ) *
                                    volumeInformation.worldSpacePerVoxel;

    lodFrustum_ = LODFrustum( frustum,
                              screenSpaceError,
                              windowHeight,
                              wsPerVoxel,
                              levelZeroNodeSize,
                              depth,
                              distances );
}

void VisibleCollectorVisitor::visit( DashRenderNode& renderNode, VisitState& state )
{
    const LODNode& lodNode = renderNode.getLODNode();
    if( !lodNode.isValid( ))
        return;

    if( lodNode.getRefLevel() == INVALID_LEVEL )
        return;

    const Boxf& worldBox = lodNode.getWorldBox();

    if( !lodFrustum_.getFrustum().boxInFrustum( worldBox) )
    {
        state.setVisitChild( false );
        return;
    }

    const int32_t index = lodNode.getMaxRefLevel() - lodNode.getRefLevel() - 1;

    if( lodFrustum_.boxInSubFrustum( worldBox, index ) )
    {
        nodeVector_.push_back( renderNode.getDashNode() );
        state.setVisitChild( false );
    }
}

class LoadedTextureCollectVisitor : public RenderNodeVisitor
{
public:

    LoadedTextureCollectVisitor( DashTreePtr dashTree,
                                 NodeIdDashNodeMap& nodeIdDashNodeMap,
                                 DashNodeVector& notAvailableRenderNodeList )
        : RenderNodeVisitor( dashTree ),
          _nodeIdDashNodeMap( nodeIdDashNodeMap ),
          _notAvailableRenderNodeList( notAvailableRenderNodeList )
    {}

    void visit( DashRenderNode& renderNode, VisitState& )
    {
        dash::NodePtr current = renderNode.getDashNode();
        while( current )
        {
            DashRenderNode currentRenderNode( current );
            const NodeId& nodeId = currentRenderNode.getLODNode().getNodeId();

            const ConstTextureObjectPtr texture =
                boost::static_pointer_cast< const TextureObject >(
                        currentRenderNode.getTextureObject( ));

            if( texture && texture->isLoaded( ))
            {
                _nodeIdDashNodeMap[ nodeId ] = current;
                break;
            }

            current = nodeId.isRoot() ? 0 : getDashTree()->getParentNode( nodeId );
        }

        if( renderNode.getDashNode() != current )
            _notAvailableRenderNodeList.push_back( renderNode.getDashNode() );
    }

private:
    NodeIdDashNodeMap& _nodeIdDashNodeMap;
    DashNodeVector& _notAvailableRenderNodeList;
};


bool hasParentInMap( DashRenderNode& childRenderNode,
                     const NodeIdDashNodeMap& nodeIdDashNodeMap )
{
    const LODNode& childNode = childRenderNode.getLODNode();

    const NodeId childNodeId = childNode.getNodeId();
    const NodeIds& parentNodeIds = childNodeId.getParents();

    BOOST_FOREACH( const NodeId& parentId, parentNodeIds )
    {
        if( nodeIdDashNodeMap.find( parentId ) != nodeIdDashNodeMap.end() )
            return true;
    }
    return false;
}

AvailableSetGenerator::AvailableSetGenerator( DashTreePtr tree,
                          const uint32_t windowHeight,
                          const float screenSpaceError )
    : RenderingSetGenerator( tree ),
      _windowHeight( windowHeight ),
      _screenSpaceError( screenSpaceError )
{
}

void AvailableSetGenerator::generateRenderingSet( const Frustum& viewFrustum,
                                        DashNodeVector& allNodesList,
                                        DashNodeVector& renderNodeList,
                                        DashNodeVector& notAvailableRenderNodeList,
                                        RenderBricks& renderBrickList )
{
    DFSTraversal dfsTraverser_;
    VisibleCollectorVisitor visibleSelector( _tree,
                                             viewFrustum,
                                             _screenSpaceError,
                                             _windowHeight,
                                             allNodesList );
    dfsTraverser_.traverse( _tree->getDataSource()->getVolumeInformation().rootNode,
                            visibleSelector );

    NodeIdDashNodeMap nodeIdDashNodeMap;

    LoadedTextureCollectVisitor collector( _tree,
                                           nodeIdDashNodeMap,
                                           notAvailableRenderNodeList );

    CollectionTraversal colTraverser;
    colTraverser.traverse( allNodesList, collector );

    NodeIdDashNodeMap::const_iterator it = nodeIdDashNodeMap.begin();
    while( it != nodeIdDashNodeMap.end() )
    {
        DashRenderNode childNode( it->second );
        if( !notAvailableRenderNodeList.empty() && hasParentInMap( childNode, nodeIdDashNodeMap ) )
        {
            it = nodeIdDashNodeMap.erase( it );
        }
        else
        {
            renderNodeList.push_back( it->second );

            const ConstTextureObjectPtr texture =
                boost::static_pointer_cast< const TextureObject >( childNode.getTextureObject( ));

            RenderBrickPtr renderBrick( new RenderBrick( texture->getLODNode(),
                                                         texture->getTextureState( )));
            renderBrickList.push_back( renderBrick );
            ++it;
        }
    }
}

}
