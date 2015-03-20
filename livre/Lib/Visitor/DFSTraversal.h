
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

#ifndef _DFSTraversal_h_
#define _DFSTraversal_h_

#include <livre/core/types.h>
#include <livre/core/dashTypes.h>
#include <livre/core/Visitor/NodeVisitor.h>

namespace livre
{

namespace detail
{
    class DFSTraversal;
}


/** Depth First Search ( DFS ) traverser for a dash node tree. */
class DFSTraversal
{
public:

    DFSTraversal();
    ~DFSTraversal();

    /**
     * Traverse the dash node tree starting from the given node.
     * @param rootNode The tree root information.
     * @param node starting node to traverse.
     * @param visitor Visitor object.
     * @return Returns true if traversal is completed, without being broken.
     */
    bool traverse( const RootNode& rootNode,
                   const NodeId& node,
                   NodeVisitor& visitor );

    /**
     * Traverse the dash node tree starting from the root.
     * @param rootNode  The tree root information.
     * @param visitor Visitor object.
     */
    void traverse( const RootNode& rootNode,
                   NodeVisitor& visitor );

private:

    detail::DFSTraversal *_impl;
};

}

#endif // _DFSTraversal_h_
