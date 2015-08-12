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

#include <livre/core/dash/DashRenderNode.h>
#include <livre/core/dash/DashRenderStatus.h>
#include <livre/core/dash/DashTree.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/data/VolumeInformation.h>
#include <livre/core/render/FrameInfo.h>
#include <livre/core/render/RenderBrick.h>
#include <livre/core/render/View.h>
#include <livre/core/visitor/RenderNodeVisitor.h>
#include <livre/lib/cache/TextureCache.h>
#include <livre/lib/cache/TextureObject.h>
#include <livre/lib/render/AvailableSetGenerator.h>
#include <livre/lib/visitor/CollectionTraversal.h>
#include <livre/lib/visitor/DFSTraversal.h>

namespace livre
{

class VisibleCollectorVisitor : public RenderNodeVisitor
{
public:
    VisibleCollectorVisitor( DashTreePtr dashTree, NodeIds& nodeIds )
        : RenderNodeVisitor( dashTree ),
          _nodeIds( nodeIds )
    {}

    void visit( DashRenderNode& renderNode, VisitState& state ) final
    {
        const LODNode& lodNode = renderNode.getLODNode();
        if( !lodNode.isValid( ))
            return;

        if( !renderNode.isInFrustum( ))
        {
            state.setVisitChild( false );
            return;
        }

        if( renderNode.isLODVisible( ))
        {
            _nodeIds.push_back( lodNode.getNodeId( ));
            state.setVisitChild( false );
        }
    }

private:
    NodeIds& _nodeIds;
};

struct AvailableSetGenerator::Impl
{
    Impl( DashTreePtr dashTree,
          const TextureCache& textureCache )
        : _dashTree( dashTree ),
          _textureCache( textureCache )
    {}

    bool hasParentInMap( const NodeId& childRenderNode,
                         const ConstCacheMap& cacheMap )
    {
        const NodeIds& parentNodeIds = childRenderNode.getParents();

        BOOST_FOREACH( const NodeId& parentId, parentNodeIds )
        {
            if( cacheMap.find( parentId.getId( )) != cacheMap.end() )
                return true;
        }
        return false;
    }

    void collectLoadedTextures( const NodeId& nodeId, ConstCacheMap& cacheMap,
                                NodeIds& notAvailableRenderNodes )
    {
        NodeId current = nodeId;
        while( current.isValid( ))
        {
            const NodeId& currentNodeId = current;
            const ConstTextureObjectPtr texture =
                boost::static_pointer_cast< const TextureObject >(
                     _textureCache.getObjectFromCache( currentNodeId.getId( )));

            if( texture && texture->isLoaded( ))
            {
                cacheMap[ currentNodeId.getId() ] = texture;
                break;
            }

            current = currentNodeId.isRoot() ? NodeId() :
                                               currentNodeId.getParent();
        }

        if( nodeId != current )
            notAvailableRenderNodes.push_back( nodeId );
    }

    void generateRenderingSet( const Frustum&, FrameInfo& frameInfo )
    {
        VisibleCollectorVisitor visibleSelector( _dashTree,
                                                 frameInfo.allNodes );
        DFSTraversal dfsTraverser;
        const RootNode& rootNode =
                _dashTree->getDataSource()->getVolumeInformation().rootNode;

        dfsTraverser.traverse( rootNode, visibleSelector,
                               _dashTree->getRenderStatus().getFrameID( ));

        ConstCacheMap cacheMap;
        for( const NodeId& nodeId : frameInfo.allNodes )
        {
            collectLoadedTextures( nodeId, cacheMap,
                                   frameInfo.notAvailableRenderNodes );
        }

        if( !frameInfo.notAvailableRenderNodes.empty( ))
        {
            ConstCacheMap::const_iterator it = cacheMap.begin();
            size_t previousSize = 0;
            do
            {
                previousSize = cacheMap.size();
                while( it != cacheMap.end( ))
                {
                    if( hasParentInMap( NodeId( it->first ), cacheMap ))
                        it = cacheMap.erase( it );
                    else
                        ++it;
                }
            }
            while( previousSize != cacheMap.size( ));
        }

        frameInfo.renderNodes.reserve( cacheMap.size( ));
        for( ConstCacheMap::const_iterator it = cacheMap.begin();
             it != cacheMap.end(); ++it )
        {
            frameInfo.renderNodes.push_back( it->second );
        }
    }

    DashTreePtr _dashTree;
    const TextureCache& _textureCache;
};


AvailableSetGenerator::AvailableSetGenerator( DashTreePtr tree,
                                              const TextureCache& textureCache )
    : RenderingSetGenerator( tree )
    , _impl( new AvailableSetGenerator::Impl( tree, textureCache ))
{
}

AvailableSetGenerator::~AvailableSetGenerator()
{
    delete _impl;
}

void AvailableSetGenerator::generateRenderingSet( const Frustum& frustum,
                                                  FrameInfo& frameInfo )
{
    _impl->generateRenderingSet( frustum, frameInfo );
}

}
