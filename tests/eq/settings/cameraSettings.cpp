/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#define BOOST_TEST_MODULE LibEq

#include <boost/test/unit_test.hpp>
#include <livre/eq/settings/CameraSettings.h>

#define TOLERANCE 0.001f

namespace ut = boost::unit_test;

BOOST_AUTO_TEST_CASE( defaultCameraSettings )
{
    livre::CameraSettings cameraSettings;

    float matrixValues[16];
    std::copy( cameraSettings.getMatrix(),
               cameraSettings.getMatrix() + 16, matrixValues );

    livre::Matrix4f identity;

    for( int i = 0; i < 16; ++i )
    {
        BOOST_CHECK_CLOSE( matrixValues[i], identity.data()[i],
                           TOLERANCE );
    }
}

BOOST_AUTO_TEST_CASE( cameraSettingsSpinModel )
{
    livre::CameraSettings cameraSettings;
    const float deltaX = 20.0f;
    const float deltaY = 20.0f;
    float correctMatrix[16] = { 0.408082f, 0.0f, 0.912945f, 0.0f,
                                0.833469f, 0.408082f, -0.372557f, 0.0f,
                                -0.372557f, 0.912945f, 0.166531f, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f };

    cameraSettings.spinModel( deltaX, deltaY );

    float matrixValues[16];
    std::copy( cameraSettings.getMatrix(),
               cameraSettings.getMatrix() + 16, matrixValues );

    for( int i = 0; i < 16; ++i )
        BOOST_CHECK_CLOSE( matrixValues[i], correctMatrix[i], TOLERANCE );
}

BOOST_AUTO_TEST_CASE( cameraSettingsMoveCamera )
{
    livre::CameraSettings cameraSettings;
    const float deltaX = 20.0f;
    const float deltaY = 20.0f;
    const float deltaZ = 20.0f;
    float correctMatrix[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                20.0f, 20.0f, 20.0f, 1.0f };

    cameraSettings.moveCamera( deltaX, deltaY, deltaZ );

    float matrixValues[16];
    std::copy( cameraSettings.getMatrix(),
               cameraSettings.getMatrix() + 16, matrixValues );

    for( int i = 0; i < 16; ++i )
        BOOST_CHECK_CLOSE( matrixValues[i], correctMatrix[i], TOLERANCE );
}

BOOST_AUTO_TEST_CASE( cameraSettingsSetCameraPosition )
{
    livre::CameraSettings cameraSettings;
    const livre::Vector3f pos( 20.0f, 20.0f, 20.0f);

    float correctMatrix[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                20.0f, 20.0f, 20.0f, 1.0f };

    cameraSettings.setCameraPosition( pos );

    float matrixValues[16];
    std::copy( cameraSettings.getMatrix(),
               cameraSettings.getMatrix() + 16, matrixValues );

    for( int i = 0; i < 16; ++i )
        BOOST_CHECK_CLOSE( matrixValues[i], correctMatrix[i], TOLERANCE );
}

BOOST_AUTO_TEST_CASE( cameraSettingsSetCameraLookat )
{
    //The default constructor puts camera at the origin.
    livre::CameraSettings cameraSettings;
    const livre::Vector3f lookat( 20.0f, 20.0f, 20.0f);

    float correctMatrix[16] = { -0.707107f, -0.408248f, -0.57735f, 0.0f,
                                0.0f, 0.816496f, -0.57735f, 0.0f,
                                0.707107f, -0.408248f, -0.57735f, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f };

    cameraSettings.setCameraLookAt( lookat );

    float matrixValues[16];
    std::copy( cameraSettings.getMatrix(),
               cameraSettings.getMatrix() + 16, matrixValues );

    for( int i = 0; i < 16; ++i )
        BOOST_CHECK_CLOSE( matrixValues[i], correctMatrix[i], TOLERANCE );
}

BOOST_AUTO_TEST_CASE( cameraSettingsEverything )
{
    livre::CameraSettings cameraSettings;
    const livre::Vector3f lookat( 13.52f, 123.53f, 21.12f);
    const livre::Vector3f pos( 0.5f, 1.17f, 6.78f );
    const float deltaX = 13.54f;
    const float deltaY = 21.49f;
    const float deltaZ = 33.25f;

    float correctMatrix[16] = { 0.413936f, -0.460246f, -0.785385f, 0.0f,
                                0.328941f, -0.728848f, 0.600482f, 0.0f,
                                -0.848796f, -0.506907f, -0.150303f, 0.0f,
                                9.3526f, 26.597f, 35.243f, 1.0f };

    cameraSettings.setCameraPosition( pos );
    cameraSettings.setCameraLookAt( lookat );
    cameraSettings.spinModel( deltaX, deltaY );
    cameraSettings.moveCamera( deltaX, deltaY, deltaZ );

    float matrixValues[16];
    std::copy( cameraSettings.getMatrix(),
               cameraSettings.getMatrix() + 16, matrixValues );

    for( int i = 0; i < 16; ++i )
        BOOST_CHECK_CLOSE( matrixValues[i], correctMatrix[i], TOLERANCE );
}
