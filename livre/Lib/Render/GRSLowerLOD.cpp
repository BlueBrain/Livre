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

#include <livre/Lib/Render/GRSLowerLOD.h>
#include <livre/Lib/Visitor/CollectionTraversal.h>
#include <livre/Lib/Visitor/DFSTraversal.h>
#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Render/RenderBrick.h>
#include <livre/Lib/Cache/TextureObject.h>
#include <livre/Lib/Algorithm/LODFrustum.h>
#include <livre/core/Data/VolumeInformation.h>

namespace livre
{

typedef boost::unordered_map< NodeId, dash::NodePtr > NodeIdDashNodeMap;

class VisibleCollectorVisitor : public NodeVisitor< dash::NodePtr >
{
public:

    VisibleCollectorVisitor( const VolumeInformation& volumeInformation,
                             const Frustum& frustum,
                             const float screenSpaceError,
                             const uint32_t windowHeight,
                             DashNodeVector& nodeVector );

    virtual void visit( dash::NodePtr dashNode, VisitState& state );

    LODFrustum lodFrustum_;
    DashNodeVector& nodeVector_;
};

VisibleCollectorVisitor::VisibleCollectorVisitor(
        const VolumeInformation& volumeInformation,
        const Frustum& frustum,
        const float screenSpaceError,
        const uint32_t windowHeight,
        DashNodeVector& nodeVector )
    : nodeVector_( nodeVector )
{
    FloatVector distances;
    distances.resize( 6, 0.0f );

    const float wsPerVoxel = volumeInformation.worldSpacePerVoxel;
    const float depth = volumeInformation.depth;
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

void VisibleCollectorVisitor::visit( dash::NodePtr dashNode, VisitState& state )
{
    DashRenderNode renderNode( dashNode );

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
        nodeVector_.push_back( dashNode );
        state.setVisitChild( false );
    }
}

class LoadedTextureCollectVisitor : public NodeVisitor< dash::NodePtr >
{
public:

    LoadedTextureCollectVisitor( NodeIdDashNodeMap& nodeIdDashNodeMap,
                                 DashNodeVector& notAvailableRenderNodeList )
        : nodeIdDashNodeMap_( nodeIdDashNodeMap ),
          notAvailableRenderNodeList_( notAvailableRenderNodeList )
    {}

    void visit( dash::NodePtr dashNode, VisitState& )
    {
        dash::NodePtr current = dashNode;
        while( current )
        {
            DashRenderNode renderNode( current );
            const ConstTextureObjectPtr texture =
                boost::static_pointer_cast< const TextureObject >( renderNode.getTextureObject( ));

            if( texture && texture->isLoaded( ))
            {
                nodeIdDashNodeMap_[ renderNode.getLODNode().getNodeId() ] =
                    current;
                break;
            }

            current = current->hasParents() ? current->getParent( 0 ) : 0;
        }

        if( dashNode != current )
            notAvailableRenderNodeList_.push_back( dashNode );
    }

private:
    NodeIdDashNodeMap& nodeIdDashNodeMap_;
    DashNodeVector& notAvailableRenderNodeList_;
};


bool hasParentInMap( DashRenderNode& childRenderNode, const NodeIdDashNodeMap& nodeIdDashNodeMap )
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

GRSLowerLOD::GRSLowerLOD( const VolumeInformation& volumeInformation,
                          dash::NodePtr tree,
                          const uint32_t windowHeight,
                          const float screenSpaceError )
    : GenerateRenderingSet( volumeInformation, tree ),
      windowHeight_( windowHeight ),
      screenSpaceError_( screenSpaceError )
{
}

void GRSLowerLOD::generateRenderingSet( const Frustum& viewFrustum,
                                        DashNodeVector& allNodesList,
                                        DashNodeVector& renderNodeList,
                                        DashNodeVector& notAvailableRenderNodeList,
                                        RenderBricks& renderBrickList )
{
    DFSTraversal dfsTraverser_;
    VisibleCollectorVisitor visibleSelector( _volumeInformation,
                                             viewFrustum,
                                             screenSpaceError_,
                                             windowHeight_,
                                             allNodesList );
    dfsTraverser_.traverse( _tree, visibleSelector );

    NodeIdDashNodeMap nodeIdDashNodeMap;

    LoadedTextureCollectVisitor collector( nodeIdDashNodeMap, notAvailableRenderNodeList );

    CollectionTraversal< const DashNodeVector > colTraverser;
    colTraverser.traverse( allNodesList, collector );

    NodeIdDashNodeMap::iterator it = nodeIdDashNodeMap.begin();
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
                    boost::static_pointer_cast< const TextureObject >( childNode.getTextureObject() );

            RenderBrickPtr renderBrick( new RenderBrick( texture->getLODNode(), texture->getTextureState() ) );
            renderBrickList.push_back( renderBrick );
            ++it;
        }
    }
}

}
