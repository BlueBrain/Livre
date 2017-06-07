/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
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

#ifndef _LODNode_h_
#define _LODNode_h_

#include <livre/data/NodeId.h>
#include <livre/data/api.h>

namespace livre
{
/**
 * Defines the spatial structure of a node in terms of world space, size and
 * voxel dimensions.
 */
class LODNode
{
public:
    /** Construct an invalid LODNode. */
    LIVREDATA_API LODNode();

    /**
     * Compute the world position from given information.
     *
     * @param nodeId Unique node id.
     * @param blockSize Block size in voxels.
     * @param levelTotalBlockDimensions Total number of blocks in the
     *                                  corresponding tree level.
     */
    LIVREDATA_API LODNode(const NodeId& nodeId, const Vector3ui& blockSize,
                          const Vector3ui& levelTotalBlockDimensions);

    /**
     * Compute the world position from given information.
     * @param nodeId Unique node id.
     * @param blockSize Block size in voxels.
     * @param worldBox Node position in the world space.
     */
    LIVREDATA_API LODNode(const NodeId& nodeId, const Vector3ui& blockSize,
                          const Boxf& worldBox);

    /**
     * Get the absolute node position wrt its level.
     *
     * The minimum position are the x,y,z absolute coordinates of the offset of
     * the given node, e.g., on level 2 they are between 0..7.
     *
     * @return Blockwise position level resolution.
     */
    LIVREDATA_API Vector3ui getAbsolutePosition() const
    {
        return _nodeId.getPosition();
    }

    /** @return the relative node position in normalized coordinates. */
    LIVREDATA_API Vector3f getRelativePosition() const;

    /**
     * @return Voxel extents in reflevel resolution.
     */
    LIVREDATA_API const Boxui& getVoxelBox() const { return _localVoxelBox; }
    /**
     * @return The box in world coordinates.
     */
    LIVREDATA_API const Boxf& getWorldBox() const { return _worldBox; }
    /**
     * @return The tree level.
     */
    LIVREDATA_API uint32_t getRefLevel() const { return _nodeId.getLevel(); }
    /**
     * @return The unique node id.
     */
    LIVREDATA_API NodeId getNodeId() const { return _nodeId; }
    /**
     * @return True if node is valid.
     */
    LIVREDATA_API bool isValid() const { return _nodeId.isValid(); }
    /**
     * @param node The node to compare.
     * @return True if nodeId's are same.
     */
    LIVREDATA_API bool operator==(const LODNode& node) const
    {
        return _nodeId == node._nodeId;
    }

    /**
     * @param parentNode Parent node.
     * @return True if the node is parent.
     */
    LIVREDATA_API bool isParent(const LODNode& parentNode) const;

    /** @return the size of the block without paddings */
    LIVREDATA_API const Vector3ui& getBlockSize() const { return _blockSize; }
private:
    const NodeId _nodeId;

    /** Number of voxels in each dimension without paddings. */
    const Vector3ui _blockSize;

    /**
     * Voxelwise position and dimension in reflevel resolution in model space.
     */
    const Boxui _localVoxelBox;

    /** World space box in normalized coordinates [ 0, 1.0 ). */
    const Boxf _worldBox;

    /**
     * Computes the internal values of LODBox (voxel extents, block extents,
     * etc).
     */
    Boxui _computeLocalBox() const;

    Boxf _computeWorldBox(const Vector3ui& levelTotalBlockDimensions) const;
};

/** Outputs the lod node information */
std::ostream& operator<<(std::ostream& os, const LODNode& lodNode);
}
#endif //_LODNode_h_
