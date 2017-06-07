
/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
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

#include <livre/data/api.h>
#include <livre/data/types.h>

namespace livre
{
/** Depth First Search ( DFS ) traverser for a LOD node tree. */
class DFSTraversal
{
public:
    LIVREDATA_API DFSTraversal();
    LIVREDATA_API ~DFSTraversal();

    /**
     * Traverse the node tree starting from the given node.
     * @param rootNode The tree root information.
     * @param node starting node to traverse.
     * @param visitor Visitor object.
     */
    LIVREDATA_API void traverse(const RootNode& rootNode, const NodeId& node,
                                NodeVisitor& visitor);

    /**
     * Traverse the node tree starting from the root.
     * @param rootNode  The tree root information.
     * @param visitor Visitor object.
     * @param timeStep The temporal position of the node tree.
     */
    LIVREDATA_API void traverse(const RootNode& rootNode, NodeVisitor& visitor,
                                const uint32_t timeStep);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};
}

#endif // _DFSTraversal_h_
