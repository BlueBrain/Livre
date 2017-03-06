/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#ifndef _NodeId_h_
#define _NodeId_h_

#include <livre/core/api.h>
#include <livre/core/defines.h>
#include <livre/core/types.h>

#include <functional>

namespace livre
{
/** Identifier for octree LOD nodes */
class NodeId
{
    /* 64 bit node id encoding */
    union {
        struct
        {
            Identifier _level : NODEID_LEVEL_BITS; //>! Maximum 15 levels
            Identifier _blockPosX
                : NODEID_BLOCK_BITS; //>! Maximum 16384 blocks in X dimension
            Identifier _blockPosY
                : NODEID_BLOCK_BITS; //>! Maximum 16384 blocks in Y dimension
            Identifier _blockPosZ
                : NODEID_BLOCK_BITS; //>! Maximum 16384 blocks in Z dimension
            Identifier _timeStep
                : NODEID_TIMESTEP_BITS; //>! Maximum 262144 frames
        };
        Identifier _id;
    };

public:
    /**
     * Constructs an invalid NodeId
     */
    LIVRECORE_API NodeId();

    /**
     * Constructs a LOD identifier
     * @param identifier Identifier number of the node
     */
    LIVRECORE_API explicit NodeId(const Identifier& identifier);

    /**
     * Constructs a LOD identifier
     * @param level Level in the octree, 0 is the coarsest level
     * @param position Position in the current level of octree
     * @param timeStep The temporal coordinate of the volume
     */
    LIVRECORE_API NodeId(const uint32_t level, const Vector3ui& position,
                         const uint32_t timeStep = 0);

    LIVRECORE_API uint32_t getLevel() const
    {
        return _level;
    } //!< The octree level
    LIVRECORE_API uint32_t getTimeStep() const
    {
        return _timeStep;
    } //!< Temporal position
    LIVRECORE_API bool isRoot() const
    {
        return _level == 0;
    }                                            //!< Is one of the root nodes
    LIVRECORE_API Vector3ui getPosition() const; //!< Return position in current
                                                 //! level of octree
    LIVRECORE_API NodeIds getParents() const;    //!< Return all parents
    LIVRECORE_API NodeId getParent() const;      //!< Return direct parent
    LIVRECORE_API bool isParent(
        const NodeId& parentNodeId) const; //!< Is parentNodeId my parent
    LIVRECORE_API bool isChild(const NodeId& childNodeId) const; //!< Is
                                                                 //! childNodeId
    //! my child
    LIVRECORE_API bool isValid() const
    {
        return _level != INVALID_LEVEL;
    }                                          //!< Is valid node id
    LIVRECORE_API NodeIds getChildren() const; //!< Returns children.
    LIVRECORE_API NodeId getRoot() const;      //!< Return root node
    LIVRECORE_API NodeIds getSiblings() const; //<! Return siblings
    LIVRECORE_API NodeIds getChildrenAtLevel(
        const uint32_t level) const;      //<! Returns children at level
    LIVRECORE_API Range getRange() const; //<! Normalized data range within tree
    LIVRECORE_API Identifier getId() const
    {
        return _id;
    } //<! Returns the unique identifier

    /**
     * @param node The node which is compared against
     * @return true if two nodes have the same id
     */
    LIVRECORE_API bool operator==(const NodeId& node) const
    {
        return _id == node._id;
    }

    /**
     * @param id The identifier which is compared against
     * @return true if the nodes have the same id
     */
    LIVRECORE_API bool operator==(const Identifier id) const
    {
        return _id == id;
    }

    /**
     * @param node The node which is compared against
     * @return false if two nodes have the same id
     */
    LIVRECORE_API bool operator!=(const NodeId& node) const
    {
        return _id != node._id;
    }

    /**
     * @param id The identifier which is compared against
     * @return false if two nodes have the same id
     */
    LIVRECORE_API bool operator!=(const Identifier id) const
    {
        return _id != id;
    } //<! Checks equality of the node

    /**
     * @param node The node which is compared against
     * @return true if node id is smaller
     */
    LIVRECORE_API bool operator<(const NodeId& node) const
    {
        return _id < node._id;
    } //<! Checks equality of the node

    /**
     * @param id The identifier which is compared against
     * @return true if id is smaller
     */
    LIVRECORE_API bool operator<(const Identifier id) const
    {
        return _id < id;
    } //<! Checks equality of the node
};

/**
 * Holds the number of levels of an LOD tree and the number of blocks at its
 * root.
 */
class RootNode
{
public:
    /**
     * @param depth the depth of the LOD tree.
     * @param blockCount the number of blocks along each axis at the root of
     *        the LOD tree.
     */
    RootNode(const uint32_t depth = 0,
             const Vector3ui& blockCount = Vector3ui(0u))
        : _treeDepth(depth)
        , _blockCount(blockCount)
    {
    }

    /**
     * @return the depth of the LOD tree.
     */
    uint32_t getDepth() const { return _treeDepth; }
    /**
     * @param level the level of the LOD tree.
     * @return the maximum number of blocks at a given level, considering a
     *         regular octree that starts from the root. This is an upper bound,
     *         which may be superior to the actual value.
     */
    Vector3ui getBlockSize(const uint32_t level = 0) const
    {
        return _blockCount * (1u << level);
    }

private:
    uint32_t _treeDepth;
    Vector3ui _blockCount;
};

inline std::ostream& operator<<(std::ostream& os, const NodeId& nodeId)
{
    return os << "Level: " << nodeId.getLevel()
              << " Position: " << nodeId.getPosition()
              << " Time Step: " << nodeId.getTimeStep();
}
}

#endif // _NodeId_h_
