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

#include <livre/core/visitor/RenderNodeVisitor.h>
#include <livre/core/dash/DashRenderNode.h>
#include <livre/core/dash/DashTree.h>

namespace livre
{

struct RenderNodeVisitor::Impl
{
public:
    explicit Impl( livre::DashTree& dashTree )
        : _dashTree( dashTree )
    {}

    dash::NodePtr getDashNode( const NodeId& nodeId )
    {
        return _dashTree.getDashNode( nodeId );
    }

    livre::DashTree& getDashTree() { return _dashTree; }
    livre::DashTree& _dashTree;
};

RenderNodeVisitor::RenderNodeVisitor( DashTree& dashTree )
    : _impl( new Impl( dashTree ))
{

}

RenderNodeVisitor::~RenderNodeVisitor()
{}

DashTree& RenderNodeVisitor::getDashTree()
{
    return _impl->getDashTree();
}

void RenderNodeVisitor::visit( const NodeId& nodeId,
                               VisitState& state )
{
    dash::NodePtr node = _impl->getDashNode( nodeId );
    if( !node)
        return;

    DashRenderNode renderNode( node );
    visit( renderNode, state );
}

}
