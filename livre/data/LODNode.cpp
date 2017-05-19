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

#include <livre/data/DataSource.h>
#include <livre/data/LODNode.h>

namespace livre
{
LODNode::LODNode()
    : _blockSize(0u)
{
}

LODNode::LODNode(const NodeId& nodeId, const Vector3ui& blockSize,
                 const Vector3ui& levelTotalBlockDimensions)
    : _nodeId(nodeId)
    , _blockSize(blockSize)
    , _localVoxelBox(_computeLocalBox())
    , _worldBox(_computeWorldBox(levelTotalBlockDimensions))
{
}

LODNode::LODNode(const NodeId& nodeId, const Vector3ui& blockSize,
                 const Boxf& worldBox)
    : _nodeId(nodeId)
    , _blockSize(blockSize)
    , _localVoxelBox(_computeLocalBox())
    , _worldBox(worldBox)
{
}

Boxf LODNode::_computeWorldBox(const Vector3ui& levelTotalBlockDimensions) const
{
    Vector3f lBoxCoordMin = getAbsolutePosition();
    Vector3f lBoxCoordMax(lBoxCoordMin + Vector3ui(1));
    const size_t index = levelTotalBlockDimensions.find_max_index();

    lBoxCoordMin = lBoxCoordMin / levelTotalBlockDimensions[index];
    lBoxCoordMax = lBoxCoordMax / levelTotalBlockDimensions[index];

    return Boxf(lBoxCoordMin, lBoxCoordMax);
}

Boxui LODNode::_computeLocalBox() const
{
    const Vector3ui& pntPos = getAbsolutePosition() * _blockSize;
    return Boxui(pntPos, pntPos + _blockSize);
}

Vector3f LODNode::getRelativePosition() const
{
    return Vector3f(getAbsolutePosition()) / float(1 << getRefLevel());
}

std::ostream& operator<<(std::ostream& os, const LODNode& lodNode)
{
    os << "Node Id: " << lodNode.getNodeId()
       << " World coords: " << lodNode.getWorldBox()
       << " Voxel box: " << lodNode.getVoxelBox()
       << " Block size: " << lodNode.getBlockSize();

    return os;
}
}
