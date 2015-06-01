/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <livre/core/Data/VolumeDataSource.h>
#include <livre/core/Data/NodeId.h>
#include <livre/core/Data/LODNode.h>
#include <livre/core/Data/MemoryUnit.h>
#include <livre/core/Data/VolumeInformation.h>
#include <livre/core/mathTypes.h>


namespace
{
const uint32_t BLOCK_SIZE = 32;
const uint32_t VOXEL_SIZE_X = 1024;
const uint32_t VOXEL_SIZE_Y = 1024;
const uint32_t VOXEL_SIZE_Z = 512;

void _testDataSource( const std::string& uriStr )
{
    const lunchbox::URI uri( uriStr );
    livre::VolumeDataSource source( uri );
    const livre::VolumeInformation& info = source.getVolumeInformation();

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

    livre::ConstLODNodePtr lodNode = source.getNode(firstChildNodeId);
    BOOST_CHECK( lodNode );
    BOOST_CHECK( lodNode->getVoxelBox().getDimension() =
                 livre::Vector3ui( BLOCK_SIZE ));

    const livre::Vector3ui blockSize = lodNode->getBlockSize() +
                                       livre::Vector3ui( info.overlap ) * 2;
    BOOST_CHECK( blockSize == info.maximumBlockSize );

    livre::MemoryUnitPtr memUnit = source.getData( *lodNode );
    const size_t allocSize = blockSize[ 0 ] * blockSize[ 1 ] * blockSize[ 2 ] *
                             info.compCount * info.getBytesPerVoxel();

    BOOST_CHECK_EQUAL( memUnit->getMemSize(), allocSize );
}
}

BOOST_AUTO_TEST_CASE( memoryDataSource )
{
    std::stringstream volumeName;
    volumeName << "mem:///#" << VOXEL_SIZE_X << "," << VOXEL_SIZE_Y << ","
               << VOXEL_SIZE_Z << "," << BLOCK_SIZE;

    _testDataSource( volumeName.str( ));
}

#ifdef LIVRE_USE_REMOTE_DATASOURCE
BOOST_AUTO_TEST_CASE( remoteMemoryDataSource )
{
    std::stringstream volumeName;
    volumeName << "remotemem:///#" << VOXEL_SIZE_X << "," << VOXEL_SIZE_Y << ","
               << VOXEL_SIZE_Z << "," << BLOCK_SIZE;
    try
    {
        _testDataSource( volumeName.str( ));
    }
    catch( const std::runtime_error& e )
    {
        const std::string error( e.what( ));
        const std::string noSource( "Cannot connect to publisher" );
        if( error != noSource )
            BOOST_CHECK_EQUAL( error,
                               std::string( "Empty servus implementation" ));
    }
}
#endif
