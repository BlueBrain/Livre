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

#include <livre/core/Data/VolumeDataSourcePlugin.h>
#include <livre/core/Data/LODNode.h>

#include <livre/core/Dash/DashRenderNode.h>

namespace livre
{

ConstLODNodePtr VolumeDataSourcePlugin::getNodeFromNodeID( const NodeId nodeId ) const
{
    CacheIDLODNodePtrMap::iterator it = _lodNodeMap.find( nodeId );
    if( it == _lodNodeMap.end( ) )
    {
        LODNodePtr lodNodePtr( new LODNode() );
        internalNodeToLODNode( nodeId, *lodNodePtr );
        _lodNodeMap[ nodeId ] = lodNodePtr;
    }

    return _lodNodeMap[ nodeId ];
}

const VolumeInformation& VolumeDataSourcePlugin::getVolumeInformation() const
{
    return _volumeInfo;
}

void initDashTree( const VolumeDataSourcePlugin* plugin,
                   const NodeId childNode,
                   dash::NodePtr destParent )
{
    if( childNode.getLevel() == plugin->getVolumeInformation().depth )
        return;

    dash::NodePtr node = new dash::Node();
    DashRenderNode::initializeDashNode( node );
    DashRenderNode dashNode( node );
    dash::Context::getCurrent().commit();
    ConstLODNodePtr lodNodePtr = plugin->getNodeFromNodeID( childNode );
    dashNode.setLODNode( *lodNodePtr );
    dash::Context::getCurrent().commit();
    destParent->insert( node );
    dash::Context::getCurrent().commit();

    const NodeIds& children = childNode.getChildren();
    BOOST_FOREACH( const NodeId childId, children )
    {
        initDashTree( plugin, childId, node );
    }
}

bool VolumeDataSourcePlugin::initializeDashTree( dash::NodePtr dashTree ) const
{
    DashRenderNode::initializeDashNode( dashTree );
    dash::Context::getCurrent().commit();
    const Vector3ui& rootDims = _volumeInfo.levelBlockDimensions.front();
    for( uint32_t x = 0; x < rootDims.x(); ++x )
        for( uint32_t y = 0; y < rootDims.y(); ++y )
            for( uint32_t z = 0; z < rootDims.x(); ++z )
                initDashTree( this, NodeId( 0, Vector3ui( x, y, z )), dashTree );

    return true;
}

void VolumeDataSourcePlugin::internalNodeToLODNode(
    const NodeId internalNode, LODNode& lodNode ) const
{
    const uint32_t refLevel = internalNode.getLevel();
    const Vector3ui& bricksInRefLevel = _volumeInfo.levelBlockDimensions[ refLevel ];
    const Boxi localBlockPos( internalNode.getPosition(),
                              internalNode.getPosition() + 1u );

    Vector3f boxCoordMin = localBlockPos.getMin();
    Vector3f boxCoordMax = localBlockPos.getMax();

    const uint32_t index = bricksInRefLevel.find_max_index( );

    boxCoordMin = boxCoordMin / bricksInRefLevel[index];
    boxCoordMax = boxCoordMax / bricksInRefLevel[index];

#ifdef LIVRE_DEBUG_RENDERING
    LBINFO << " Internal Node to LOD Node" << std::endl
           << " Node Id " << internalNode
           << " BricksInRefLevel " << bricksInRefLevel << std::endl
           << " lBoxCoordMin " << boxCoordMin << std::endl
           << " lBoxCoordMax " << boxCoordMax << std::endl
           << " volume world size " << _volumeInfo.worldSize << std::endl
           << std::endl;
#endif

    lodNode = LODNode( internalNode,
                       _volumeInfo.depth,
                       _volumeInfo.maximumBlockSize - _volumeInfo.overlap * 2,
                       Boxf( boxCoordMin - _volumeInfo.worldSize * 0.5f,
                             boxCoordMax - _volumeInfo.worldSize * 0.5f ) );
}

bool isPowerOfTwo( const uint32_t x )
{
    return ( x != 0 ) && ( ( x & ( x - 1 ) ) == 0 );
}

bool checkCompatibility( const Vector3ui& volumeDim,
                         const Vector3ui& blockSize )
{
    if( volumeDim.x() % blockSize.x() )
        return false;
    if( volumeDim.y() % blockSize.y() )
        return false;
    if( volumeDim.z() % blockSize.z() )
        return false;

    if( !isPowerOfTwo( volumeDim.x() / blockSize.x() ) )
        return false;
    if( !isPowerOfTwo( volumeDim.y() / blockSize.y() ) )
        return false;
    if( !isPowerOfTwo( volumeDim.z() / blockSize.z() ) )
        return false;

    return true;
}

bool fillRegularVolumeInfo( VolumeInformation& info )
{
    const Vector3ui& blockSize = info.maximumBlockSize - info.overlap * 2;
    if( !checkCompatibility( info.voxels, blockSize ))
        return false;

    const float maxDim = float( info.voxels.find_max( ));

    info.maximumBlockSize = blockSize + info.overlap * 2;
    info.dataType = DT_UINT8;
    info.compCount = 1;
    info.isBigEndian = false;
    info.worldSpacePerVoxel = 1.0f / maxDim;
    info.worldSize = Vector3f( info.voxels[0], info.voxels[1],
                                info.voxels[2] ) / maxDim;

    // Find the depth of hierarchy
    const Vector3ui numBlocks = info.voxels / blockSize;
    Vector3ui blocksSize = numBlocks;

    const uint32_t xDepth = std::log2( blocksSize[0] );
    const uint32_t yDepth = std::log2( blocksSize[1] );
    const uint32_t zDepth = std::log2( blocksSize[2] );

    info.depth = std::min( xDepth, std::min( yDepth, zDepth )) + 1;

    // Set max, min values and generate the node arrays for each tree level
    info.levelBlockDimensions.resize( info.depth );

    blocksSize = numBlocks;
    for( uint32_t i = 0; i < info.depth; ++i )
    {
        const uint32_t level = info.depth - i - 1;
        info.levelBlockDimensions[ level ] = blocksSize;
        blocksSize = blocksSize / 2;
    }

    return true;
}



}
