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

#ifndef LIVRE_DATASERVICE_H
#define LIVRE_DATASERVICE_H

#include <livre/core/Data/LODNode.h>
#include <livre/core/Data/VolumeDataSource.h>
#include <livre/Lib/zeq/events.h>
#include <zeq/zeq.h>
#include <lunchbox/stdExt.h>

namespace livre
{
/**
 * Service providing data to a livre::remote::DataSource.
 *
 * This is the service part, typically running on a remote system serving data
 * to a Livre application using a remote data source. Runs a broker on *:27766
 * to allow remote, non-zeroconf connections.
 *
 * Example: @include tests/remote/dataSource.cpp
 */
class DataService
{
public:
    DataService()
        : _publisher( lunchbox::URI( "livresource://" ))
        , _subscriber( lunchbox::URI( "livresink://" ))
        , _broker( "*:27766", _subscriber )
        , _running( true )
    {
        _setup();
    }

    explicit DataService( const std::string& address )
        : _publisher( lunchbox::URI( "livresource://" + address ))
        , _subscriber( lunchbox::URI( "livresink://" + address ))
        , _broker( "*:27766", _subscriber )
        , _running( true )
    {
        _setup();
    }

    ~DataService() { VolumeDataSource::unloadPlugins(); }

    bool isRunning() const { return _running; }
    void processOne() { _subscriber.receive(); }

private:
    typedef stde::hash_map< ::zeq::uint128_t,
                            VolumeDataSourcePtr > DataSourceMap;
    ::zeq::Publisher _publisher;
    ::zeq::Subscriber _subscriber;
    ::zeq::connection::Broker _broker;
    DataSourceMap _dataSources;
    bool _running;

    void _setup()
    {
        VolumeDataSource::loadPlugins();

        if( !_subscriber.registerHandler( zeq::EVENT_DATASOURCE,
                    boost::bind( &livre::DataService::_newClient, this, _1 )) ||
            !_subscriber.registerHandler( ::zeq::vocabulary::EVENT_EXIT,
                           boost::bind( &livre::DataService::_stop, this, _1 )))
        {
            LBTHROW( std::runtime_error(
                         "ZeroEQ event handler registration failed" ));
        }

        LBINFO << "Livre data service set up" << std::endl;
    }

    void _stop( const ::zeq::Event& ) { _running = false; }

    void _newClient( const ::zeq::Event& event )
    {
        const lunchbox::URI& uri = zeq::deserializeDataSource( event );
        const std::string& uriStr = boost::lexical_cast< std::string >( uri );
        const ::zeq::uint128_t& dataEvent = lunchbox::make_uint128(
            std::string( "livre::zeq::" ) + uriStr );

        VolumeDataSourcePtr dataSource = _dataSources[ dataEvent ];
        if( !dataSource )
        {
            dataSource.reset( new VolumeDataSource( uri ));
            if( !dataSource )
            {
                LBWARN << "Data source " << uriStr << " initialization failed"
                       << std::endl;
                return;
            }

            _dataSources[ dataEvent ] = dataSource;
            LBCHECK( _subscriber.registerHandler( dataEvent,
                                          boost::bind( &DataService::_newSample,
                                                       this, _1 )));
        }

        const VolumeInformation& info=dataSource->getVolumeInformation();
        _publisher.publish( zeq::serializeDataSourceData(
                                std::make_pair( dataEvent, info )));
        LBINFO << "Serving " << uri << std::endl;
    }

    void _newSample( const ::zeq::Event& event )
    {
        const zeq::LODNodeSample& request = zeq::deserializeDataSample( event );
        VolumeDataSourcePtr dataSource = _dataSources[ request.first ];
        if( !dataSource )
            return;

        MemoryUnitPtr data = dataSource->getData( request.second );
        _publisher.publish( zeq::serializeDataSampleData( request.first + 1
                                                          , data ));
        std::cout << '.' << std::flush;
    }
};
}

#endif
