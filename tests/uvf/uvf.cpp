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

#define BOOST_TEST_MODULE UVFDataSource
#include <boost/test/unit_test.hpp>

#ifdef LIVRE_USE_TUVOK

#include <livre/core/data/DataSource.h>
#include <livre/core/data/LODNode.h>

const uint32_t BLOCK_SIZE = 28;
const uint32_t OVERLAP_SIZE = 2;
const uint32_t VOXEL_SIZE_X = 75;
const uint32_t VOXEL_SIZE_Y = 75;
const uint32_t VOXEL_SIZE_Z = 138;

BOOST_AUTO_TEST_CASE( UVFDataSource )
{
    std::cout << "uvf://" UVF_DATA_FILE << std::endl;
    const lunchbox::URI uri( "uvf://" UVF_DATA_FILE );
    livre::DataSource source( uri );
    const livre::VolumeInformation& info = source.getVolumeInfo();

    BOOST_CHECK( info.rootNode.getDepth() == 2 );
    BOOST_CHECK( info.compCount == 1 );
    BOOST_CHECK( info.dataType == livre::DT_UINT8 );
    BOOST_CHECK( info.voxels == livre::Vector3ui( VOXEL_SIZE_X,
                                                  VOXEL_SIZE_Y,
                                                  VOXEL_SIZE_Z ));

    BOOST_CHECK( info.overlap == livre::Vector3ui( OVERLAP_SIZE ));

    const uint32_t level = 0;
    const livre::Vector3f position( 0, 0, 0 );
    const uint32_t frame = 0;
    const livre::NodeId parentNodeId( level, position, frame );
    const livre::NodeId firstChildNodeId =
        parentNodeId.getChildren().front();

    const livre::LODNode& lodNode = source.getNode( firstChildNodeId );
    BOOST_CHECK( lodNode.isValid( ));
    BOOST_CHECK_EQUAL( lodNode.getVoxelBox().getSize(),
                       livre::Vector3ui( BLOCK_SIZE ));

    const livre::Vector3ui& blockSize = lodNode.getBlockSize() +
                                       livre::Vector3ui( info.overlap ) * 2;
    BOOST_CHECK( blockSize == info.maximumBlockSize );

    livre::MemoryUnitPtr memUnit = source.getData( firstChildNodeId );
    const size_t allocSize = blockSize.product() *
                             info.compCount * info.getBytesPerVoxel();

    BOOST_CHECK_EQUAL( memUnit->getMemSize(), allocSize );
}
#else
BOOST_AUTO_TEST_CASE( UVFDataSource ) {}
#endif // LIVRE_USE_TUVOK
