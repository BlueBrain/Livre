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

#include <livre/core/data/DataSource.h>
#include <livre/core/render/SelectVisibles.h>
#include <livre/core/visitor/DFSTraversal.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/ClipPlanes.h>

#include <boost/test/unit_test.hpp>

typedef std::vector< livre::Identifier > Identifiers;

Identifiers getVisibles( const livre::DataSource& dataSource,
                         const uint32_t windowHeight,
                         const float screenSpaceError,
                         const uint32_t minLOD,
                         const uint32_t maxLOD )
{
    const float projArray[] = { 2.0, 0, 0, 0,
                                0, 2.0, 0, 0,
                                0, 0, -1.01342285, -1,
                                0, 0, -0.201342285, 0 };

    const livre::Matrix4f projMat( projArray, projArray + 16 );

    const float mvArray[] = { 1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, -1.0, 1 };

    const livre::Matrix4f mvMat( mvArray, mvArray + 16 );
    const livre::Frustum frustum( mvMat, projMat );

    livre::ClipPlanes planes;
    livre::SelectVisibles selectVisibles( dataSource,
                                          frustum,
                                          windowHeight,
                                          screenSpaceError,
                                          minLOD,
                                          maxLOD,
                                          {{ 0.0f, 1.0f }},
                                          planes );

    livre::DFSTraversal traverser;
    traverser.traverse( dataSource.getVolumeInfo().rootNode,
                        selectVisibles, 0 );


    Identifiers visibles;
    for( const livre::NodeId& nodeId: selectVisibles.getVisibles( ))
        visibles.push_back( nodeId.getId( ));

    std::sort( visibles.begin(), visibles.end( ));
    return visibles;
}

BOOST_AUTO_TEST_CASE( testLODSelection )
{
    const lunchbox::URI uri( "mem://#4096,4096,4096,256" );
    livre::DataSource dataSource( uri );

    {
        const Identifiers& visibles = getVisibles( dataSource, 256, 1.0, 0, 100 );
        const Identifiers results = { 1, 17, 262145, 262161, 8589934594, 8589934610,
                                       8589934626, 8589934642, 8590196738, 8590196754,
                                       8590196770, 8590196786, 8590458882, 8590458898,
                                       8590458914, 8590458930, 8590721026, 8590721042,
                                       8590721058, 8590721074, 12884901890, 12884901906,
                                       12884901922, 12884901938, 12885164034, 12885164050,
                                       12885164066, 12885164082, 12885426178, 12885426194,
                                       12885426210, 12885426226, 12885688322, 12885688338,
                                       12885688354, 12885688370 };

        BOOST_CHECK_EQUAL_COLLECTIONS( results.data(), results.data() + results.size(),
                                       visibles.data(), visibles.data() + visibles.size( ));
    }

    {
        const Identifiers& visibles = getVisibles( dataSource, 256, 2.0, 0, 100 );
        const Identifiers results = { 1, 17, 262145, 262161, 4294967297, 4294967313, 4295229441,
                                       4295229457 };

        BOOST_CHECK_EQUAL_COLLECTIONS( results.data(), results.data() + results.size(),
                                       visibles.data(), visibles.data() + visibles.size( ));
    }

    {
        const Identifiers& visibles = getVisibles( dataSource, 256, 8.0, 0, 100 );
        const Identifiers results = { 0 };

        BOOST_CHECK_EQUAL_COLLECTIONS( results.data(), results.data() + results.size(),
                                       visibles.data(), visibles.data() + visibles.size( ));
    }

    {
        const Identifiers& visibles = getVisibles( dataSource, 512, 1.0, 0, 100 );
        const Identifiers results = { 1, 17, 262145, 262161, 8589934594, 8589934610, 8589934626,
                                      8589934642, 8590196738, 8590196754, 8590196770, 8590196786,
                                      8590458882, 8590458898, 8590458914, 8590458930, 8590721026,
                                      8590721042, 8590721058, 8590721074, 25769803779, 25769803795,
                                      25769803811, 25769803827, 25769803843, 25769803859, 25769803875,
                                      25769803891, 25770065923, 25770065939, 25770065955, 25770065971,
                                      25770065987, 25770066003, 25770066019, 25770066035, 25770328067,
                                      25770328083, 25770328099, 25770328115, 25770328131, 25770328147,
                                      25770328163, 25770328179, 25770590211, 25770590227, 25770590243,
                                      25770590259, 25770590275, 25770590291, 25770590307, 25770590323,
                                      25770852355, 25770852371, 25770852387, 25770852403, 25770852419,
                                      25770852435, 25770852451, 25770852467, 25771114499, 25771114515,
                                      25771114531, 25771114547, 25771114563, 25771114579, 25771114595,
                                      25771114611, 25771376643, 25771376659, 25771376675, 25771376691,
                                      25771376707, 25771376723, 25771376739, 25771376755, 25771638787,
                                      25771638803, 25771638819, 25771638835, 25771638851, 25771638867,
                                      25771638883, 25771638899, 30065033235, 30065033251, 30065033267,
                                      30065033283, 30065033299, 30065033315, 30065295379, 30065295395,
                                      30065295411, 30065295427, 30065295443, 30065295459, 30065557523,
                                      30065557539, 30065557555, 30065557571, 30065557587, 30065557603,
                                      30065819667, 30065819683, 30065819699, 30065819715, 30065819731,
                                      30065819747, 30066081811, 30066081827, 30066081843, 30066081859,
                                      30066081875, 30066081891, 30066343955, 30066343971, 30066343987,
                                      30066344003, 30066344019, 30066344035 };

        BOOST_CHECK_EQUAL_COLLECTIONS( results.data(), results.data() + results.size(),
                                       visibles.data(), visibles.data() + visibles.size( ));

    }

    {
        const Identifiers& visibles = getVisibles( dataSource, 512, 2.0, 0, 100 );
        const Identifiers results = { 1, 17, 262145, 262161, 8589934594, 8589934610, 8589934626,
                                      8589934642, 8590196738, 8590196754, 8590196770, 8590196786,
                                      8590458882, 8590458898, 8590458914, 8590458930, 8590721026,
                                      8590721042, 8590721058, 8590721074, 12884901890, 12884901906,
                                      12884901922, 12884901938, 12885164034, 12885164050, 12885164066,
                                      12885164082, 12885426178, 12885426194, 12885426210, 12885426226,
                                      12885688322, 12885688338, 12885688354, 12885688370 };

        BOOST_CHECK_EQUAL_COLLECTIONS( results.data(), results.data() + results.size(),
                                       visibles.data(), visibles.data() + visibles.size( ));

    }

    {
        const Identifiers& visibles = getVisibles( dataSource, 512, 8.0, 0, 100 );
        const Identifiers results = { 0 };

        BOOST_CHECK_EQUAL_COLLECTIONS( results.data(), results.data() + results.size(),
                                       visibles.data(), visibles.data() + visibles.size( ));
    }


    {
        const uint32_t maxMinLevel = 0;
        const Identifiers& visibles = getVisibles( dataSource, 512, 1.0, maxMinLevel, maxMinLevel );
        const Identifiers results = { 0 };

        BOOST_CHECK_EQUAL_COLLECTIONS( results.data(), results.data() + results.size(),
                                       visibles.data(), visibles.data() + visibles.size( ));

        for( const livre::Identifier& visible: visibles )
            BOOST_CHECK_EQUAL( livre::NodeId( visible ).getLevel(), maxMinLevel );

    }

    {
        const uint32_t maxMinLevel = 1;
        const Identifiers& visibles = getVisibles( dataSource, 512, 1.0, maxMinLevel, maxMinLevel );
        const Identifiers results = { 1, 17, 262145, 262161, 4294967297, 4294967313, 4295229441,
                                      4295229457 };

        BOOST_CHECK_EQUAL_COLLECTIONS( results.data(), results.data() + results.size(),
                                       visibles.data(), visibles.data() + visibles.size( ));

        for( const livre::Identifier& visible: visibles )
            BOOST_CHECK_EQUAL( livre::NodeId( visible ).getLevel(), maxMinLevel );
    }
}
