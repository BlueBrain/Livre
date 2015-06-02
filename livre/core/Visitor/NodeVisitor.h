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

#ifndef _NodeVisitor_h_
#define _NodeVisitor_h_

#include <livre/core/types.h>

namespace livre
{

/**
 * The NodeVisitor class is base class for invoking traversing events. Some of the events may not be
 * implemented by the traverser.
  */
class NodeVisitor
{
public:

    NodeVisitor( )
    { }

    /**
     * Is called at the beginning of a traversal.
     * @param state Visit state.
     */
    virtual void onTraverseBegin( VisitState& state LB_UNUSED )
    { }

    /**
     * Is called at the end of a traversal.
     * @param state Visit state.
     */
    virtual void onTraverseEnd( const VisitState& state LB_UNUSED )
    { }

    /**
     * Called before the node is  visited.
     * @param renderNode is the node being visited.
     * @param state Visit state.
     */
    virtual void onVisitBegin( const NodeId& nodeId LB_UNUSED,
                               const VisitState& state LB_UNUSED )
    { }

    /**
     * Called after the node has been visited.
     * @param renderNode is the node being visited.
     * @param state Visit state.
     */
    virtual void onVisitEnd( const NodeId& nodeId LB_UNUSED,
                             const VisitState& state LB_UNUSED )
    { }

    /**
     * Called before all the children of the node is visited.
     * @param renderNode is the node being visited.
     * @param state Visit state.
     */
    virtual void onVisitChildrenBegin( const NodeId& nodeId LB_UNUSED,
                                       const VisitState& state LB_UNUSED )
    { }

    /**
     * Called after all the children of the node has been visited.
     * @param renderNode is the node being visited.
     * @param state Visit state.
     */
    virtual void onVisitChildrenEnd( const NodeId& nodeId LB_UNUSED,
                                     const VisitState& state LB_UNUSED )
    { }

    /**
     * Called when the given node is being visited.
     * @param renderNode is the node being visited.
     * @param state Visit state.
     */
    virtual void visit( const NodeId& nodeId,
                        VisitState& state ) = 0;

    virtual ~NodeVisitor( ) { }

};

}

#endif // _NodeVisitor_h_
