/* Copyright (c) 2014, EPFL/Blue Brain Project
 *                     Stefan.Eilemann@epfl.ch
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

#define BOOST_TEST_MODULE LibEvent
#include <boost/test/unit_test.hpp>

#include <livre/Lib/zeq/events.h>
#include <livre/core/Data/VolumeInformation.h>
#include <zeq/event.h>
#include <zeq/publisher.h>
#include <zeq/subscriber.h>
#include <lunchbox/rng.h>
#include <lunchbox/uri.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

using boost::lexical_cast;
using livre::Vector3ui;
using livre::Vector3f;

const unsigned short port = (lunchbox::RNG().get<uint16_t>() % 60000) + 1024;
const std::string& portStr = lexical_cast< std::string >( port );
const lunchbox::URI uri( "livreTest://localhost:" + portStr );

void onDataSourceEvent( const zeq::Event& event )
{
    BOOST_CHECK( event.getType() == livre::zeq::EVENT_DATASOURCE );
    const lunchbox::URI& deserialized =
        livre::zeq::deserializeDataSource( event );
    BOOST_CHECK_EQUAL( lexical_cast< std::string >( uri ),
                       lexical_cast< std::string >( deserialized ));
}

BOOST_AUTO_TEST_CASE( testDataSource )
{
    zeq::Publisher publisher( lunchbox::URI( "livreTest://*:" + portStr ));
    zeq::Subscriber subscriber( uri );
    BOOST_CHECK( subscriber.registerHandler( livre::zeq::EVENT_DATASOURCE,
                                             boost::bind( &onDataSourceEvent,
                                                          _1 )));
    bool received = false;
    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK(
            publisher.publish( livre::zeq::serializeDataSource( uri )));

        if( subscriber.receive( 100 ))
        {
            received = true;
            break;
        }
    }
    BOOST_CHECK( received );
}

void onDataSourceDataEvent( const zeq::Event& event,
                            const livre::VolumeInformation& info )
{
    BOOST_CHECK( event.getType() == livre::zeq::EVENT_DATASOURCE_DATA );
    const livre::zeq::RemoteInformation& deserialized =
        livre::zeq::deserializeDataSourceData( event );
    const livre::VolumeInformation& info2 = deserialized.second;

    BOOST_CHECK_EQUAL( deserialized.first, zeq::uint128_t( 17, 42 ));
    BOOST_CHECK_EQUAL( info.compCount, info2.compCount );
    BOOST_CHECK_EQUAL( info.rootNode.getDepth(), info2.rootNode.getDepth( ));
    BOOST_CHECK_EQUAL( info.overlap, info2.overlap );
    BOOST_CHECK_EQUAL( info.maximumBlockSize, info2.maximumBlockSize );
    BOOST_CHECK_EQUAL( info.minPos, info2.minPos );
    BOOST_CHECK_EQUAL( info.maxPos, info2.maxPos );
    BOOST_CHECK_EQUAL( info.worldSize, info2.worldSize );
    BOOST_CHECK_EQUAL( info.voxels, info2.voxels );
    BOOST_CHECK_EQUAL( info.worldSpacePerVoxel, info2.worldSpacePerVoxel );
}

BOOST_AUTO_TEST_CASE( testDataSourceData )
{
    zeq::Publisher publisher( lunchbox::URI( "livreTest://*:" + portStr ));
    zeq::Subscriber subscriber( uri );
    livre::VolumeInformation info;
    lunchbox::RNG rng;

    info.compCount = rng.get<uint32_t>();
    info.overlap = Vector3ui( rng.get<uint32_t>(), rng.get<uint32_t>(),
                              rng.get<uint32_t>( ));
    info.maximumBlockSize = Vector3ui( rng.get<uint32_t>(), rng.get<uint32_t>(),
                                       rng.get<uint32_t>( ));
    info.minPos = Vector3f( rng.get<float>(), rng.get<float>(),
                             rng.get<float>( ));
    info.maxPos = Vector3f( rng.get<float>(), rng.get<float>(),
                            rng.get<float>( ));
    info.worldSize = Vector3f( rng.get<float>(), rng.get<float>(),
                               rng.get<float>( ));
    info.voxels = Vector3ui( rng.get<uint32_t>(), rng.get<uint32_t>(),
                             rng.get<uint32_t>( ));
    info.worldSpacePerVoxel = rng.get<float>();

    info.rootNode = livre::RootNode( rng.get<uint32_t>(), Vector3ui( 1, 2, 3 ));

    BOOST_CHECK( subscriber.registerHandler( livre::zeq::EVENT_DATASOURCE_DATA,
                                            boost::bind( &onDataSourceDataEvent,
                                                         _1, info )));
    bool received = false;
    for( size_t i = 0; i < 10; ++i )
    {
        BOOST_CHECK(
            publisher.publish( livre::zeq::serializeDataSourceData(
                                   std::make_pair( zeq::uint128_t( 17, 42 ),
                                                   info ))));

        if( subscriber.receive( 100 ))
        {
            received = true;
            break;
        }
    }
    BOOST_CHECK( received );
}
