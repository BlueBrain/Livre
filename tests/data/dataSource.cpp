/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#define BOOST_TEST_MODULE DataSource
#include <boost/test/unit_test.hpp>

#include <livre/core/data/DataSource.h>
#include <livre/core/data/NodeId.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/data/VolumeInformation.h>

namespace
{
const uint32_t BLOCK_SIZE = 32;
const uint32_t VOXEL_SIZE_X = 1024;
const uint32_t VOXEL_SIZE_Y = 1024;
const uint32_t VOXEL_SIZE_Z = 512;

void _testDataSource( const std::string& uriStr )
{
    const lunchbox::URI uri( uriStr );
    livre::DataSource source( uri );
    const livre::VolumeInformation& info = source.getVolumeInfo();

     // 32 << depth = 512 - shortest dimension of the volume.
    BOOST_CHECK( info.rootNode.getDepth() == 5 );

    // Component count per voxel, intensity only 1 channels.
    BOOST_CHECK( info.compCount == 1 );

    const uint32_t level = 0;
    const livre::Vector3f position( 0, 0, 0);
    const uint32_t frame = 0;
    const livre::NodeId parentNodeId( level, position, frame );
    const livre::NodeId firstChildNodeId =
        parentNodeId.getChildren().front();

    const livre::LODNode& lodNode = source.getNode( firstChildNodeId );
    BOOST_CHECK( lodNode.isValid( ));
    BOOST_CHECK( lodNode.getVoxelBox().getSize() =
                 livre::Vector3ui( BLOCK_SIZE ));

    const livre::Vector3ui blockSize = lodNode.getBlockSize() +
                                       livre::Vector3ui( info.overlap ) * 2;
    BOOST_CHECK( blockSize == info.maximumBlockSize );
}
}

BOOST_AUTO_TEST_CASE( memoryDataSource )
{
    std::stringstream volumeName;
    volumeName << "mem://#" << VOXEL_SIZE_X << "," << VOXEL_SIZE_Y << ","
               << VOXEL_SIZE_Z << "," << BLOCK_SIZE;

    _testDataSource( volumeName.str( ));
}
