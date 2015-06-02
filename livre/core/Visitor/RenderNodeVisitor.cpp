/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <livre/core/Visitor/RenderNodeVisitor.h>
#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Dash/DashTree.h>

namespace livre
{

namespace detail
{

class RenderNodeVisitor
{
public:
    RenderNodeVisitor(  DashTreePtr dashTree )
        : _dashTree( dashTree )
    {}

    dash::NodePtr getDashNode( const NodeId& nodeId )
    {
        return _dashTree->getDashNode( nodeId );
    }

    DashTreePtr getDashTree() { return _dashTree; }
    DashTreePtr _dashTree;
};

}

RenderNodeVisitor::RenderNodeVisitor( DashTreePtr dashTree )
    : _impl( new detail::RenderNodeVisitor( dashTree ))
{

}

RenderNodeVisitor::~RenderNodeVisitor()
{
    delete _impl;
}

DashTreePtr RenderNodeVisitor::getDashTree()
{
    return _impl->getDashTree();
}

void RenderNodeVisitor::onVisitBegin( const NodeId& nodeId,
                                      const VisitState& state )
{
    DashRenderNode renderNode( _impl->getDashNode( nodeId ));
    onVisitBegin( renderNode, state );
}

void RenderNodeVisitor::onVisitEnd( const NodeId& nodeId,
                                    const VisitState& state )
{
    DashRenderNode renderNode( _impl->getDashNode( nodeId ));
    onVisitEnd( renderNode, state );
}

void RenderNodeVisitor::onVisitChildrenBegin(
                           const NodeId& nodeId,
                           const VisitState& state )
{
    DashRenderNode renderNode( _impl->getDashNode( nodeId ));
    onVisitChildrenBegin( renderNode, state );
}

void RenderNodeVisitor::onVisitChildrenEnd( const NodeId& nodeId,
                                            const VisitState& state )
{
    DashRenderNode renderNode( _impl->getDashNode( nodeId ));
    onVisitChildrenEnd( renderNode, state );
}

void RenderNodeVisitor::visit( const NodeId& nodeId,
                               VisitState& state )
{
    DashRenderNode renderNode( _impl->getDashNode( nodeId ));
    visit( renderNode, state );
}

}


