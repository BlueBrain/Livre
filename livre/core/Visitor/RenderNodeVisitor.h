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

#ifndef _RenderNodeVisitor_h_
#define _RenderNodeVisitor_h_

#include <livre/core/types.h>
#include <livre/core/Visitor/NodeVisitor.h>

namespace livre
{

namespace detail
{
class RenderNodeVisitor;
}

/**
 * The RenderNodeVisitor class is base class for invoking traversing events on DashRenderNodes.
*/
class RenderNodeVisitor : public NodeVisitor
{

public:

    RenderNodeVisitor( DashTreePtr dashTree );
    ~RenderNodeVisitor();

    /**
     * @see NodeVisitor::onVisitBegin
     */
    virtual void onVisitBegin( DashRenderNode& node LB_UNUSED,
                               const VisitState& state LB_UNUSED)
    {}

    /**
     * @see NodeVisitor::onVisitEnd
     */
    virtual void onVisitEnd( DashRenderNode& node LB_UNUSED,
                             const VisitState& state LB_UNUSED)
    {}

    /**
     * @see NodeVisitor::onVisitChildrenBegin
     */
    virtual void onVisitChildrenBegin( DashRenderNode& node LB_UNUSED,
                                       const VisitState& state LB_UNUSED)
    {}

    /**
     * @see NodeVisitor::onVisitChildrenEnd
     */
    virtual void onVisitChildrenEnd( DashRenderNode& node LB_UNUSED,
                                     const VisitState& state LB_UNUSED)
    {}

    /**
     * @see NodeVisitor::visit
     */
    virtual void visit( DashRenderNode& node,
                        VisitState& state ) = 0;

    /**
     * @return Returns the dash tree
     */
    DashTreePtr getDashTree();

private:
    void onVisitBegin( const NodeId& nodeId, const VisitState& state ) final;
    void onVisitEnd( const NodeId& nodeId, const VisitState& state ) final;
    void onVisitChildrenBegin( const NodeId& nodeId,
                               const VisitState& state) final;
    void onVisitChildrenEnd( const NodeId& nodeId,
                             const VisitState& state ) final;
    void visit( const NodeId& nodeId, VisitState& state ) final;

    detail::RenderNodeVisitor* _impl;
};

}

#endif // _RenderNodeVisitor_h_
