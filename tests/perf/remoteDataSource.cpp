/* Copyright (c) 2014-2015, EPFL/Blue Brain Project
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

// Can be invoked manually to be run against a pre-started livreService on
// another host:
// ./remote_dataSource 'remotemem://remoteIP:remotePort/?bind=localIP'

#define BOOST_TEST_MODULE RemoteDataSource
#include <boost/test/unit_test.hpp>

#ifdef LIVRE_USE_REMOTE_DATASOURCE
#include <apps/livreService/dataService.h>
#include <livre/remote/dataSource.h>
#include <lunchbox/clock.h>
#include <lunchbox/thread.h>

static const std::string port = boost::lexical_cast< std::string >(
    (lunchbox::RNG().get< uint16_t >() % 60000) + 1024 );

class DataService : public lunchbox::Thread
{
public:
    DataService( const lunchbox::URI& uri )
        : _service(( uri.findQuery( "bind" ) == uri.queryEnd() ?
                     std::string( "127.0.0.1" ) :
                     uri.findQuery( "bind" )->second ) + ":" + port )
    {}

private:
    livre::DataService _service;

    void run() final
    {
        while( _service.isRunning( ))
            _service.processOne();
    }
};

using boost::lexical_cast;

BOOST_AUTO_TEST_CASE( testRemote )
{
    const int argc = boost::unit_test::framework::master_test_suite().argc;
    char** argv = boost::unit_test::framework::master_test_suite().argv;
    const bool externalSource = argc == 2;
    const std::string uriBase = externalSource ? argv[1] :
                                        ("remotemem://127.0.0.1:" + port + "/");

    zeq::Publisher publisher( lunchbox::URI( "livresink://" ));
    DataService* service = 0;

    if( !externalSource )
    {
        service = new DataService( lunchbox::URI( uriBase ));
        service->start();
    }

    lunchbox::Strings result;
    try
    {
        for( size_t i = 32; i < 1024; i = i << 1 )
        {
            lunchbox::Clock clock;
            livre::VolumeDataSource dataSource(
                lunchbox::URI( uriBase + "#1024,1024,1024," +
                               lexical_cast< std::string >( i )));
            const float setup = clock.getTimef();

            const livre::VolumeInformation& info =
                dataSource.getVolumeInformation();
            const vmml::Vector3i block( i );
            livre::Boxf worldBox( vmml::Vector3i( 0.f ) ,
                                  vmml::Vector3i( 1.f ));
            livre::LODNode node( livre::NodeId( 0, block ),
                                 info.rootNode.getDepth() /*maxLevel*/,
                                 block,
                                 worldBox );
            clock.reset();
            livre::MemoryUnitPtr mem = dataSource.getData( node );
            const float first = clock.getTimef();

            const livre::Vector3ui brickSize = livre::Vector3ui( i ) +
                                               info.overlap * 2;
            BOOST_CHECK( mem );
            BOOST_CHECK_EQUAL( mem->getMemSize(), brickSize.product( ));

            const uint8_t* bytes = mem->getData< uint8_t >();
            for( size_t j = 0; j < mem->getMemSize(); ++j )
                BOOST_CHECK_EQUAL( bytes[j], 42 );

            size_t num = 0;
            clock.reset();
            while( clock.getTimef() < 300.f )
            {
                mem = dataSource.getData( node );
                ++num;

                BOOST_CHECK( mem );
                BOOST_CHECK_EQUAL( mem->getMemSize(), brickSize.product( ));
            }
            const float bulk = clock.getTimef();

            bytes = mem->getData< uint8_t >();
            for( size_t j = 0; j < mem->getMemSize(); ++j )
                BOOST_CHECK_MESSAGE( bytes[j] == 42,
                                     "Invalid data at byte " << j );

            std::ostringstream os;
            os << i << ", " << setup << ", "
               << mem->getMemSize()/1024.f*1000.f/1024.f / first << ", "
               << mem->getMemSize()/1024.f*1000.f/1024.f / bulk * num;
            result.push_back( os.str( ));
        }
        std::cout << std::endl
                  << "Volume size, setup (ms), MB/s (first brick),  MB/s (other bricks)"
                  << std::endl;
        BOOST_FOREACH( const std::string& string, result )
            std::cout << string << std::endl;
    }
    catch( const std::runtime_error& e )
    {
        BOOST_CHECK_EQUAL( e.what(),
                           std::string( "Cannot connect to publisher" ));
        if( service )
            // Cannot connect, so EVENT_EXIT will never be received
            service->cancel();
        return;
    }

    if( service )
    {
        BOOST_CHECK( publisher.publish(
                         zeq::Event( zeq::vocabulary::EVENT_EXIT )));
        service->join();
        delete service;
    }
}
#else
BOOST_AUTO_TEST_CASE( testRemote )
{
    BOOST_CHECK_EQUAL( 1, 1 );
}
#endif
