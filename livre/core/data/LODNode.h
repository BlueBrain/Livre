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

#ifndef _LODNode_h_
#define _LODNode_h_

#include <livre/core/api.h>
#include <livre/core/data/NodeId.h>
#include <livre/core/mathTypes.h>

namespace livre
{

/**
 * The LODNode class is to define spatial structure of a node in terms of world space, size and voxel dimensions.
 */
class LODNode
{
public:
    /** Construct an invalid LODNode. */
    LIVRECORE_API LODNode();

    /**
     * Compute the world position from given information.
     *
     * @param nodeId Unique node id.
     * @param blockSize Block size in voxels.
     * @param levelTotalBlockDimensions Total number of blocks in the
     *                                  corresponding tree level.
     */
    LIVRECORE_API LODNode( const NodeId nodeId,
                           const Vector3i& blockSize,
                           const Vector3i& levelTotalBlockDimensions );

    /**
     * Compute the world position from given information.
     * @param nodeId Unique node id.
     * @param blockSize Block size in voxels.
     * @param worldBox Node position in the world space.
     */
    LIVRECORE_API LODNode( const NodeId nodeId,
                           const Vector3i& blockSize,
                           const Boxf& worldBox );

    /**
     * Get the absolute node position wrt its level.
     *
     * The minimum position are the x,y,z absolute coordinates of the offset of
     * the given node, e.g., on level 2 they are between 0..7.
     *
     * @return Blockwise position level resolution.
     */
    LIVRECORE_API Vector3i getAbsolutePosition() const { return nodeId_.getPosition(); }

    /** @return the relative node position in normalized coordinates. */
    LIVRECORE_API Vector3f getRelativePosition() const;

    /**
     * @return Voxel extents in reflevel resolution.
     */
    LIVRECORE_API const Boxui& getVoxelBox( ) const { return localVoxelBox_; }

    /**
     * @return The box in world coordinates.
     */
    LIVRECORE_API const Boxf& getWorldBox( ) const { return worldBox_; }

    /**
     * @return The tree level.
     */
    LIVRECORE_API int32_t getRefLevel( ) const { return nodeId_.getLevel(); }

    /**
     * @return The unique node id.
     */
    LIVRECORE_API NodeId getNodeId( ) const { return nodeId_; }

    LIVRECORE_API const Vector3i& getBlockSize() const { return blockSize_; } //!< @internal

    /**
     * @return True if node is valid.
     */
    LIVRECORE_API bool isValid( ) const { return nodeId_.isValid(); }

    /**
     * @param node The node to compare.
     * @return True if nodeId's are same.
     */
    LIVRECORE_API bool operator==( const LODNode& node ) const {  return nodeId_ == node.nodeId_; }

    /**
     * @param parentNode Parent node.
     * @return True if the node is parent.
     */
    LIVRECORE_API bool isParent( const LODNode& parentNode ) const;

    /**
     * Single empty node.
     */
    static LODNode empty;

private:

    NodeId nodeId_; //!< Node id.
    Vector3i blockSize_; //!< Number of blocks in each dimension.
    Boxui localVoxelBox_; //!< Voxelwise position and dimension in reflevel resolution in model space.
    Boxf worldBox_; //!< World box.
    void initialize_( ); //!< Computes the internal values of LODBox (voxel extents, block extents, etc ).
    void computeWorldBox_( const Vector3ui& levelTotalBlockDimensions ); //!< compute world box.
};

}
#endif //_LODNode_h_
