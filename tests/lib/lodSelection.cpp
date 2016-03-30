/* Copyright (c) 2016, EPFL/Blue Brain Project
 *                     ahmet.bilgili@epfl.ch
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

#define BOOST_TEST_MODULE LodSelection
#include <boost/test/unit_test.hpp>

#include <livre/core/render/Frustum.h>
#include <livre/core/maths/maths.h>

uint32_t getLOD( const uint32_t minLOD,
                 const uint32_t maxLOD,
                 const float screenSpaceError,
                 const livre::Vector3f& worldPos,
                 const float cameraDist )
{

    const uint32_t windowHeight = 600;
    const size_t maxVolumeVoxelSize = 2048;
    const float worldSpacePerVoxel = 1.0f / maxVolumeVoxelSize;
    const uint32_t volumeDepth = 8;
    const float projArray[] = { 1.24999988, 0, 0, 0,
                                0, 2.0, 0, 0,
                                0, 0, -1.01342285, -1,
                                0, 0, -0.201342285, 0 };

    const livre::Matrix4f projMat( projArray, projArray + 16 );

    const float mvArray[] = { 1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, -cameraDist, 1 };
    const livre::Matrix4f mvMat( mvArray, mvArray + 16 );

    const livre::Frustum frustum( mvMat, projMat );

    return livre::maths::getLODForPoint( frustum,
                                         worldPos,
                                         worldSpacePerVoxel,
                                         volumeDepth,
                                         windowHeight,
                                         screenSpaceError,
                                         minLOD,
                                         maxLOD );
}

BOOST_AUTO_TEST_CASE( testLODSelection )
{

    const uint32_t sse0 = 1;
    // Test position is in front of the near plane ( near plane = 0.1 )
    BOOST_CHECK( getLOD( 0, 10, sse0, livre::Vector3f( 0.5, -0.5, 0.5 ), 1 ) == 7 );
    // Test position  position is far from the near plane
    BOOST_CHECK( getLOD( 0, 10, sse0, livre::Vector3f( 0.5, -0.5, 0.5 ), 4 ) == 4 );
    // Test if test is done against near plane
    BOOST_CHECK( getLOD( 0, 10, sse0, livre::Vector3f( 0, 0, 0.499999911 ), 1 ) == 7 );
    // Test if max and min lod constrained
    BOOST_CHECK( getLOD( 0, 1, sse0, livre::Vector3f( 0, 0, 0.499999911 ), 1 ) == 1 );
    BOOST_CHECK( getLOD( 0, 0, sse0, livre::Vector3f( 0, 0, 0.499999911 ), 1 ) == 0 );

    const uint32_t sse1 = 4;
    // Test position is in front of the near plane ( near plane = 0.1 )
    BOOST_CHECK( getLOD( 0, 10, sse1, livre::Vector3f( 0.5, -0.5, 0.5 ), 1 ) == 5 );
    // Test position  position is far from the near plane
    BOOST_CHECK( getLOD( 0, 10, sse1, livre::Vector3f( 0.5, -0.5, 0.5 ), 4 ) == 2 );
    // Test if test is done against near plane
    BOOST_CHECK( getLOD( 0, 10, sse1, livre::Vector3f( 0, 0, 0.499999911 ), 1 ) == 5 );
    // Test if max and min lod constrained
    BOOST_CHECK( getLOD( 0, 1, sse1, livre::Vector3f( 0, 0, 0.499999911 ), 1 ) == 1 );
    BOOST_CHECK( getLOD( 0, 0, sse1, livre::Vector3f( 0, 0, 0.499999911 ), 1 ) == 0 );

    const uint32_t sse2 = 16;
    // Test position is in front of the near plane ( near plane = 0.1 )
    BOOST_CHECK( getLOD( 0, 10, sse2, livre::Vector3f( 0.5, -0.5, 0.5 ), 1 ) == 3);
    // Test position  position is far from the near plane
    BOOST_CHECK( getLOD( 0, 10, sse2, livre::Vector3f( 0.5, -0.5, 0.5 ), 4 ) == 0);
    // Test if test is done against near plane
    BOOST_CHECK( getLOD( 0, 10, sse2, livre::Vector3f( 0, 0, 0.499999911  ), 1 ) == 3 );
    // Test if max and min lod constrained
    BOOST_CHECK( getLOD( 0, 1, sse2, livre::Vector3f( 0, 0, 0.499999911 ), 1 ) == 1 );
    BOOST_CHECK( getLOD( 0, 0, sse2, livre::Vector3f( 0, 0, 0.499999911 ), 1 ) == 0 );
}
