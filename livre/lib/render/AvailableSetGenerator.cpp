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

#include <livre/core/render/RenderBrick.h>
#include <livre/core/render/View.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/FrameInfo.h>
#include <livre/core/render/LODEvaluator.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/data/VolumeInformation.h>
#include <livre/core/visitor/RenderNodeVisitor.h>
#include <livre/core/visitor/VisitState.h>
#include <livre/core/visitor/CollectionTraversal.h>
#include <livre/core/visitor/DFSTraversal.h>

#include <livre/lib/render/AvailableSetGenerator.h>
#include <livre/lib/cache/TextureObject.h>
#include <livre/lib/cache/TextureCache.h>

namespace livre
{

struct AvailableSetGenerator::Impl
{
    Impl( const TextureCache& textureCache )
        : _textureCache( textureCache )
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

    void collectLoadedTextures(
                const NodeId& nodeId,
                ConstCacheMap& cacheMap,
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

            current = currentNodeId.isRoot() ? NodeId() : currentNodeId.getParent();
        }

        if( nodeId != current )
            notAvailableRenderNodes.push_back( nodeId );
    }

    void generateRenderingSet( FrameInfo& frameInfo,
                               const NodeIds& allVisibleNodes )
    {


        BOOST_FOREACH( const NodeId& nodeId, allVisibleNodes )
            collectLoadedTextures( nodeId,
                                   _cacheMap,
                                   frameInfo.notAvailableRenderNodes );

        if( !frameInfo.notAvailableRenderNodes.empty( ))
        {
            ConstCacheMap::const_iterator it = _cacheMap.begin();
            size_t previousSize = 0;
            do
            {
                previousSize = _cacheMap.size();
                while( it != _cacheMap.end( ))
                {
                    if( hasParentInMap( NodeId( it->first ), _cacheMap ))
                        it = _cacheMap.erase( it );
                    else
                        ++it;
                }
            }
            while( previousSize != _cacheMap.size( ));
        }

        frameInfo.renderNodes.reserve( _cacheMap.size( ));
        for( ConstCacheMap::const_iterator it = _cacheMap.begin();
             it != _cacheMap.end(); ++it )
        {
            frameInfo.renderNodes.push_back( it->second );
        }
    }

    const TextureCache& _textureCache;
    ConstCacheMap _cacheMap;
};


AvailableSetGenerator::AvailableSetGenerator( const TextureCache& textureCache )
    : _impl( new AvailableSetGenerator::Impl( textureCache ))
{
}

AvailableSetGenerator::~AvailableSetGenerator()
{

}

void AvailableSetGenerator::generateRenderingSet( FrameInfo& frameInfo,
                                                  const NodeIds& allVisibleNodes )
{
    _impl->generateRenderingSet( frameInfo,
                                 allVisibleNodes );
}

}
