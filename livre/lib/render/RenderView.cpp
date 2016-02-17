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

#include <livre/lib/render/RenderView.h>
#include <livre/lib/cache/TextureObject.h>
#include <livre/lib/visitor/DFSTraversal.h>

#include <livre/core/dash/DashRenderNode.h>
#include <livre/core/dash/DashTree.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/data/VolumeInformation.h>
#include <livre/core/maths/maths.h>
#include <livre/core/render/FrameInfo.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/GLWidget.h>

namespace livre
{

struct RenderView::Impl
{
    explicit Impl( ConstDashTreePtr dashTree ) : _dashTree( dashTree ) {}

    void freeTexture( const NodeId& nodeId )
    {
        dash::NodePtr dashNode = _dashTree->getDashNode( nodeId );
        if( !dashNode )
            return;

        DashRenderNode renderNode( dashNode );
        if( renderNode.getLODNode().getRefLevel() != 0 )
            renderNode.setTextureObject( CacheObjectPtr( ));
    }

    void freeTextures( const FrameInfo& frameInfo )
    {
        for( const ConstCacheObjectPtr& cacheObject:
                       frameInfo.renderNodes )
        {
            const NodeId nodeId(cacheObject->getId( ));
            freeTexture( nodeId );
        }

        for( const NodeId& nodeId: frameInfo.allNodes )
            freeTexture( nodeId );
    }

    ConstDashTreePtr _dashTree;

};

RenderView::RenderView( ConstDashTreePtr dashTree )
    : _impl( new RenderView::Impl( dashTree ))
{
}

RenderView::~RenderView()
{
    delete _impl;
}

void RenderView::_onPostRender( const GLWidget&,
                                const FrameInfo& frameInfo )
{
    _impl->freeTextures( frameInfo );
}


}
