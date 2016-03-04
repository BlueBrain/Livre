/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Raphael Dumusc <raphael.dumusc@epfl.ch>
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

#define BOOST_TEST_MODULE LibCore

#include <boost/test/unit_test.hpp>

#include "livre/core/data/VolumeInformation.h"
#include "livre/core/data/DataSourcePlugin.h"
#include "livre/core/data/LODNode.h"

namespace ut = boost::unit_test;

livre::VolumeInformation makeTestVolumeInformation()
{
    livre::VolumeInformation volume;
    volume.maximumBlockSize = livre::Vector3ui( 64u );
    volume.overlap = livre::Vector3ui( 0u );
    volume.voxels = livre::Vector3ui( 2048u );
    volume.frameRange = livre::Vector2ui( 0u, 1u );
    return volume;
}

BOOST_AUTO_TEST_CASE( defaultVolumeInformation )
{
    livre::VolumeInformation volume;

    BOOST_CHECK_EQUAL( volume.isBigEndian, false );
    BOOST_CHECK_EQUAL( volume.compCount, 1u );
    BOOST_CHECK_EQUAL( volume.dataType, livre::DT_UINT8 );
    BOOST_CHECK_EQUAL( volume.overlap, livre::Vector3ui( 0u ));
    BOOST_CHECK_EQUAL( volume.maximumBlockSize, livre::Vector3ui( 0u ));
    BOOST_CHECK_EQUAL( volume.minPos, livre::Vector3f( 0.f ));
    BOOST_CHECK_EQUAL( volume.maxPos, livre::Vector3f( 0.f ));
    BOOST_CHECK_EQUAL( volume.voxels, livre::Vector3ui( 256u ));
    BOOST_CHECK_EQUAL( volume.worldSize, livre::Vector3f( 0.f ));
    BOOST_CHECK_EQUAL( volume.boundingBox, livre::Boxf::makeUnitBox( ));
    BOOST_CHECK_EQUAL( volume.worldSpacePerVoxel, 0.f );
    BOOST_CHECK_EQUAL( volume.frameRange, livre::INVALID_FRAME_RANGE );
}

BOOST_AUTO_TEST_CASE( regularOctree )
{
    livre::VolumeInformation volume = makeTestVolumeInformation();
    livre::fillRegularVolumeInfo( volume );
    BOOST_CHECK_EQUAL( volume.worldSpacePerVoxel, 1.f / 2048.f );
    BOOST_CHECK_EQUAL( volume.worldSize, livre::Vector3f( 1.f ));
    BOOST_CHECK_EQUAL( volume.rootNode.getDepth(), 6u );
    BOOST_CHECK_EQUAL( volume.rootNode.getBlockSize(), livre::Vector3ui( 1u ));
}

BOOST_AUTO_TEST_CASE( irregularOctrees )
{
    livre::VolumeInformation volume = makeTestVolumeInformation();
    volume.voxels[1] = 2099u;
    livre::fillRegularVolumeInfo( volume );
    BOOST_CHECK_EQUAL( volume.rootNode.getDepth(), 6u );
    BOOST_CHECK_EQUAL( volume.rootNode.getBlockSize(),
                       livre::Vector3ui( 1u, 2u, 1u ));
    BOOST_CHECK_EQUAL( volume.worldSpacePerVoxel, 1.f / 2099.f );
    BOOST_CHECK_EQUAL( volume.worldSize, livre::Vector3f( 2048.f / 2099.f,
                                                          1.f,
                                                          2048.f / 2099.f ));

    volume.voxels[1] = 2037u;
    livre::fillRegularVolumeInfo( volume );
    BOOST_CHECK_EQUAL( volume.rootNode.getDepth(), 6u );
    BOOST_CHECK_EQUAL( volume.rootNode.getBlockSize(), livre::Vector3ui( 1u ));
    BOOST_CHECK_EQUAL( volume.worldSpacePerVoxel, 1.f / 2048.f );
    BOOST_CHECK_EQUAL( volume.worldSize, livre::Vector3f( 1.f,
                                                          2037.f / 2048.f,
                                                          1.f ));
}

BOOST_AUTO_TEST_CASE( nonOctreeLODTree )
{
    // This highlights a known corner case, where the parent-child relationship
    // is lost for a border portion of the octree.
    // During the creation of the lod tree, a dimension of 2049 is downsampled
    // to 1024 which causes the number of 64^3 blocks to jump from 33 to 16.
    // The 33rd block, which only contain 64x64x1 voxels of data, has no parent
    // in the livre octree (which is built top-down instead of bottom-up) and
    // will never be rendered.
    livre::VolumeInformation volume = makeTestVolumeInformation();
    volume.voxels[1] = 2049u;

    livre::fillRegularVolumeInfo( volume );
    BOOST_CHECK_EQUAL( volume.rootNode.getDepth(), 6u );
    BOOST_CHECK_EQUAL( volume.rootNode.getBlockSize(), livre::Vector3ui( 1u ));
    BOOST_CHECK_EQUAL( volume.worldSpacePerVoxel, 1.f / 2049.f );
    BOOST_CHECK_EQUAL( volume.worldSize, livre::Vector3f( 2048.f / 2049.f,
                                                          1.f,
                                                          2048.f / 2049.f ));
}
