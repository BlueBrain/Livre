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

#include <livre/core/data/DataSourcePlugin.h>

namespace livre
{
DataSourcePlugin::DataSourcePlugin()
    : _lodNodeMap(128)
{
}

LODNode DataSourcePlugin::getNode(const NodeId& nodeId) const
{
    IdLODNodeMap::iterator it;
    {
        ReadLock lock(_mutex);
        it = _lodNodeMap.find(nodeId.getId());
        if (it != _lodNodeMap.end())
            return it->second;
    }

    WriteLock writeLock(_mutex);
    it = _lodNodeMap.find(nodeId.getId());
    if (it == _lodNodeMap.end())
    {
        const LODNode& node = internalNodeToLODNode(nodeId);
        _lodNodeMap[nodeId.getId()] = node;
    }

    return _lodNodeMap[nodeId.getId()];
}

const VolumeInformation& DataSourcePlugin::getVolumeInfo() const
{
    return _volumeInfo;
}

LODNode DataSourcePlugin::internalNodeToLODNode(
    const NodeId& internalNode) const
{
    const uint32_t refLevel = internalNode.getLevel();
    const Vector3ui& bricksInRefLevel =
        _volumeInfo.rootNode.getBlockSize(refLevel);
    const Boxi localBlockPos(internalNode.getPosition(),
                             internalNode.getPosition() + 1u);

    Vector3f boxCoordMin = localBlockPos.getMin();
    Vector3f boxCoordMax = localBlockPos.getMax();

    const size_t index = bricksInRefLevel.find_max_index();

    boxCoordMin = boxCoordMin / bricksInRefLevel[index];
    boxCoordMax = boxCoordMax / bricksInRefLevel[index];

    LBVERB << " Internal Node to LOD Node" << std::endl
           << " Node Id " << internalNode << " BricksInRefLevel "
           << bricksInRefLevel << std::endl
           << " lBoxCoordMin " << boxCoordMin << std::endl
           << " lBoxCoordMax " << boxCoordMax << std::endl
           << " volume world size " << _volumeInfo.worldSize << std::endl
           << std::endl;

    return LODNode(internalNode,
                   _volumeInfo.maximumBlockSize - _volumeInfo.overlap * 2,
                   Boxf(boxCoordMin * _volumeInfo.worldSize -
                            _volumeInfo.worldSize * 0.5f,
                        boxCoordMax * _volumeInfo.worldSize -
                            _volumeInfo.worldSize * 0.5f));
}

bool fillRegularVolumeInfo(VolumeInformation& info)
{
    info.worldSpacePerVoxel = 1.0f / float(info.voxels.find_max());
    info.worldSize = Vector3f(info.voxels[0], info.voxels[1], info.voxels[2]) *
                     info.worldSpacePerVoxel;

    // Create the rootNode of the LOD hierarchy
    const Vector3ui blockSize = info.maximumBlockSize - info.overlap * 2;
    const Vector3ui numBlocks(std::ceil(float(info.voxels.x()) / blockSize.x()),
                              std::ceil(float(info.voxels.y()) / blockSize.y()),
                              std::ceil(float(info.voxels.z()) /
                                        blockSize.z()));
    const Vector3ui lodLevels(std::ceil(std::log2(numBlocks.x())),
                              std::ceil(std::log2(numBlocks.y())),
                              std::ceil(std::log2(numBlocks.z())));
    const uint32_t depth = lodLevels.find_min();
    const Vector3ui rootNodeBlocksCount(
        std::ceil(float(info.voxels.x() >> depth) / blockSize.x()),
        std::ceil(float(info.voxels.y() >> depth) / blockSize.y()),
        std::ceil(float(info.voxels.z() >> depth) / blockSize.z()));
    info.rootNode = RootNode(depth + 1, rootNodeBlocksCount);
    return true;
}
}
