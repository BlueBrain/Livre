/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Jafet.VillafrancaDiaz@epfl.ch
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

#include <livre/core/util/FrameUtils.h>

#include <boost/test/unit_test.hpp>

bool checkFrameRange( const livre::Vector2ui& frameRange,
                      const livre::Vector2ui& boundaries,
                      const livre::Vector2ui& expected )
{
    livre::FrameUtils frameUtils( frameRange, boundaries );
    return frameUtils.getFrameRange() == expected;
}

BOOST_AUTO_TEST_CASE( testFrameRange )
{
    // within the boundaries
    livre::Vector2ui frameRange( 8, 16 );
    livre::Vector2ui boundaries( 0, 23 );
    livre::Vector2ui expected( 8, 16 );
    BOOST_CHECK( checkFrameRange( frameRange, boundaries, expected ));

    // frame range wider than boundaries
    frameRange = { 0, 23 };
    boundaries = { 8, 16 };
    expected = { 8, 16 };
    BOOST_CHECK( checkFrameRange( frameRange, boundaries, expected ));

    // outside the boundaries at the beginning
    frameRange = { 0, 16 };
    boundaries = { 8, 23 };
    expected = { 8, 16 };
    BOOST_CHECK( checkFrameRange( frameRange, boundaries, expected ));

    // outside the boundaries at the end
    frameRange = { 8, 23 };
    boundaries = { 0, 16 };
    expected = { 8, 16 };
    BOOST_CHECK( checkFrameRange( frameRange, boundaries, expected ));

    // totally outside the boundaries
    frameRange = { 0, 8 };
    boundaries = { 16, 23 };
    expected = livre::INVALID_FRAME_RANGE;
    BOOST_CHECK( checkFrameRange( frameRange, boundaries, expected ));

    frameRange = { 16, 23 };
    boundaries = { 0, 8 };
    expected = livre::INVALID_FRAME_RANGE;
    BOOST_CHECK( checkFrameRange( frameRange, boundaries, expected ));
}

BOOST_AUTO_TEST_CASE( testCurrentFrame )
{
    livre::Vector2ui frameRange( 1, 4 );
    livre::Vector2ui boundaries( 1, 4 );
    livre::FrameUtils frameUtils( frameRange, boundaries );

    BOOST_CHECK_EQUAL( frameUtils.getCurrent( 1 ), 1 );
    BOOST_CHECK_EQUAL( frameUtils.getCurrent( 0 ), 1 );
    BOOST_CHECK_EQUAL( frameUtils.getCurrent( 5 ), 3 );
    BOOST_CHECK_EQUAL( frameUtils.getCurrent( 4 ), 3 );
}

BOOST_AUTO_TEST_CASE( testCurrentFrameInvalidFrame )
{
    // INVALID_FRAME as input
    livre::Vector2ui frameRange( 0, 4 );
    livre::Vector2ui boundaries( 0, 4 );
    livre::FrameUtils frameUtils( frameRange, boundaries );
    BOOST_CHECK_EQUAL( frameUtils.getCurrent( livre::INVALID_FRAME ), 0 );

    frameRange = { 2, 4 };
    frameUtils = livre::FrameUtils( frameRange, boundaries );
    BOOST_CHECK_EQUAL( frameUtils.getCurrent( livre::INVALID_FRAME ), 2 );

    // INVALID_FRAME as output (querying an INVALID_FRAME_RANGE)
    frameRange = { 0, 8 };
    boundaries = { 16, 23 };
    frameUtils = livre::FrameUtils( frameRange, boundaries );
    BOOST_REQUIRE_EQUAL( frameUtils.getFrameRange(), livre::INVALID_FRAME_RANGE );
    BOOST_CHECK_EQUAL( frameUtils.getCurrent( 0 ), livre::INVALID_FRAME );
    BOOST_CHECK_EQUAL( frameUtils.getCurrent( 16 ), livre::INVALID_FRAME );
}

BOOST_AUTO_TEST_CASE( testCurrentFrameLatest )
{
    const bool latestAlways = true;
    livre::Vector2ui frameRange( 1, 4 );
    livre::FrameUtils frameUtils( frameRange, frameRange );
    BOOST_CHECK_EQUAL( frameUtils.getCurrent( 1, latestAlways ), 3 );
    BOOST_CHECK_EQUAL( frameUtils.getCurrent( 0, latestAlways ), 3 );
    BOOST_CHECK_EQUAL( frameUtils.getCurrent( 5, latestAlways ), 3 );
    BOOST_CHECK_EQUAL( frameUtils.getCurrent( 4, latestAlways ), 3 );
}

BOOST_AUTO_TEST_CASE( testNextFrameNumber )
{
    livre::Vector2ui frameRange( 0, 4 );
    livre::FrameUtils frameUtils( frameRange, frameRange );
    BOOST_CHECK_EQUAL( frameUtils.getNext( 0, 1 ), 1 );
    BOOST_CHECK_EQUAL( frameUtils.getNext( 1, 2 ), 3 );
    BOOST_CHECK_EQUAL( frameUtils.getNext( 2, -1 ), 1 );
    BOOST_CHECK_EQUAL( frameUtils.getNext( 0, 4 ), 0 );
    BOOST_CHECK_EQUAL( frameUtils.getNext( 3, 2 ), 1 );
}

BOOST_AUTO_TEST_CASE( testNextFrameNumberInvalidFrame )
{
    // INVALID_FRAME as output (querying an INVALID_FRAME_RANGE)
    livre::Vector2ui frameRange = { 0, 8 };
    livre::Vector2ui boundaries = { 16, 23 };
    livre::FrameUtils frameUtils( frameRange, boundaries );
    BOOST_REQUIRE_EQUAL( frameUtils.getFrameRange(), livre::INVALID_FRAME_RANGE );
    BOOST_CHECK_EQUAL( frameUtils.getNext( 0, 1 ), livre::INVALID_FRAME );
    BOOST_CHECK_EQUAL( frameUtils.getNext( 16 , 1 ), livre::INVALID_FRAME );
}
