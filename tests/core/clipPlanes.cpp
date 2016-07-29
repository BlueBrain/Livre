/* Copyright (c) 2016, EPFL/Blue Brain Project
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

#define BOOST_TEST_MODULE ClipPlanes

#include <livre/core/render/ClipPlanes.h>
#include <livre/core/mathTypes.h>

#include <vmmlib/aabb.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( testClipping )
{
    // The default clipping planes include space defined by AABB
    // ( -0.5, -0.5, -0.5 ) to ( 0.5, 0.5, 0.5 )

    const livre::Vector3f cornerOut1( 0.8f, 0.8f, 0.8f );
    const livre::Vector3f cornerOut2( 0.9f, 0.9f, 0.9f );
    const livre::Vector3f cornerIn1( -0.3f, -0.3f, -0.3f );
    const livre::Vector3f cornerIn2(  0.3f, 0.3f, 0.3f );

    const livre::Boxf boxInside( cornerIn1, cornerIn2 );
    const livre::Boxf boxOutside( cornerOut1, cornerOut2 );
    const livre::Boxf boxIntersect( cornerIn1, cornerOut2 );

    livre::ClipPlanes clipPlanes;

    BOOST_CHECK( !clipPlanes.isClipped( boxInside ));
    BOOST_CHECK( clipPlanes.isClipped( boxOutside ));
    BOOST_CHECK( !clipPlanes.isClipped( boxIntersect ));

    BOOST_CHECK( !clipPlanes.isEmpty( ));
    clipPlanes.clear();
    BOOST_CHECK( clipPlanes.isEmpty( ));

    clipPlanes.reset();
    BOOST_CHECK( !clipPlanes.isEmpty( ));

    BOOST_CHECK( !clipPlanes.isClipped( boxInside ));
    BOOST_CHECK( clipPlanes.isClipped( boxOutside ));
    BOOST_CHECK( !clipPlanes.isClipped( boxIntersect ));
}
