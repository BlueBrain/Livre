
/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                          Ahmet.Bilgili@epfl.ch
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

#ifndef _BFSTraversal_h_
#define _BFSTraversal_h_

#include <livre/core/types.h>
#include <livre/core/dashTypes.h>
#include <livre/core/Visitor/NodeVisitor.h>
#include <livre/core/Visitor/VisitState.h>

namespace livre
{

/**
 * @brief The BFSTraversal class is the Breadth First Search ( BFS ) traverser for a dash node tree.
 */
class BFSTraversal
{
public:

    /**
     * @brief BFSTraversal constructor.
     */
    BFSTraversal();

    /**
     * @brief traverse Starts the traversing of dash node tree.
     * @param root the root of the tree to traverse.
     * @param visitor Visitors visit is revoked for each visit of a dash node.
     * @return Returns true if traversal is completed, without being broken.
     */
    bool traverse( dash::NodePtr root, NodeVisitor< dash::NodePtr >& visitor );

private:
    VisitState state_; //!< Status of the travel
};

}

#endif // _BFSTraversal_h_
