
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

#include "dataSource.h"

#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>

#include <livre/lib/zeq/events.h>

#include <zeq/event.h>
#include <zeq/publisher.h>
#include <zeq/subscriber.h>
#include <zeq/connection/service.h>

#include <boost/bind.hpp>
#include <lunchbox/pluginRegisterer.h>

namespace livre
{
namespace remote
{
using boost::lexical_cast;

namespace detail
{
static const uint32_t timeout = 60000; /*ms*/
namespace
{
lunchbox::URI _getSinkURI( const VolumeDataSourcePluginData& initData )
{
    const lunchbox::URI& initURI = initData.getURI();
    lunchbox::URI::ConstKVIter i = initURI.findQuery( "bind" );
    if( i == initURI.queryEnd( ))
        return lunchbox::URI( "livresink://" );
    return lunchbox::URI( "livresink://" + i->second );
}

lunchbox::URI _getSourceURI( const VolumeDataSourcePluginData& initData )
{
    if( initData.getURI().getHost().empty( ))
        return lunchbox::URI( "livresource://" );
    if( initData.getURI().getPort() == 0 )
        return lunchbox::URI( "livresource://" + initData.getURI().getHost( ));
    return lunchbox::URI( "livresource://" + initData.getURI().getHost() + ":" +
                    lexical_cast< std::string >( initData.getURI().getPort( )));
}
}

class DataSource
{
public:
    DataSource( const VolumeDataSourcePluginData& initData,
                VolumeInformation& info )
        : _publisher( _getSinkURI( initData ))
        , _subscriber( _getSourceURI( initData ))
    {
        const std::string& host = initData.getURI().getHost();
        if( !host.empty( ))
            ::zeq::connection::Service::subscribe( host+":27766", _publisher );

        lunchbox::URI uri( initData.getURI() );
        uri.setScheme( uri.getScheme().substr( ::strlen( "remote" )));

        if( !_subscriber.registerHandler( livre::zeq::EVENT_DATASOURCE_DATA,
                       boost::bind( &livre::remote::detail::DataSource::_onInfo,
                                    this, _1, boost::ref( info ))))
        {
            LBTHROW( std::runtime_error( "Cannot register subscriber") );
        }

        std::cout << "Looking for remote " << uri.getScheme() << " data source";
        size_t tries = 1 << 8;
        const uint32_t rTimeout = timeout >> 8;
        while( _event == 0 && --tries )
        {
            LBCHECK( _publisher.publish( zeq::serializeDataSource( uri )));
            std::cout << "." << std::flush;
            _subscriber.receive( rTimeout );
        }
        std::cout << (_event != 0 ? "found" : "timeout") << std::endl;

        LBCHECK(
            _subscriber.deregisterHandler( livre::zeq::EVENT_DATASOURCE_DATA ));
        if( tries == 0 )
            LBTHROW( std::runtime_error( "Cannot connect to publisher") );
    }


    MemoryUnitPtr sample( const LODNode& node )
    {
        lunchbox::Clock clock;

        // ZeroEQ/MQ is not thread safe. Livre calls this concurrently from all
        // render threads. When using a thread-save ZeroEQ, use node id in place
        // of '+ 1' to have one handler per brick.
        lunchbox::ScopedWrite mutex( _lock );
        AllocMemoryUnit* memory = 0;

        LBCHECK( _subscriber.registerHandler( _event + 1,
                       boost::bind( &livre::remote::detail::DataSource::_onData,
                                    this, _1, boost::ref( memory ))));
        LBCHECK( _publisher.publish( zeq::serializeDataSample( _event, node )));

        while( !memory )
        {
            const float left = timeout - clock.getTimef();
            if( left <= 0.f )
                break;
            _subscriber.receive( left );
        }

        LBCHECK( _subscriber.deregisterHandler( _event + 1 ));
        return AllocMemoryUnitPtr( memory );
    }

private:
    ::zeq::Publisher _publisher;
    ::zeq::Subscriber _subscriber;
    lunchbox::uint128_t _event;
    lunchbox::Lock _lock;

    void _onInfo( const ::zeq::Event& event, VolumeInformation& info )
    {
        const livre::zeq::RemoteInformation& data =
            livre::zeq::deserializeDataSourceData( event );

        _event = data.first;
        info = data.second;
    }

    void _onData( const ::zeq::Event& event, AllocMemoryUnit*& ptr )
    {
        AllocMemoryUnit* memory = new AllocMemoryUnit;
        const livre::zeq::LODNodeSampleData& data =
            livre::zeq::deserializeDataSampleData( event );
        memory->allocAndSetData( data.first, data.second );
        ptr = memory;
    }
};
}

namespace
{
   lunchbox::PluginRegisterer< DataSource > registerer;
}

DataSource::DataSource()
     : _impl( 0 )
{}

//TODO: generalize URI and rm this ctor once plugins are used
DataSource::DataSource( const VolumeDataSourcePluginData& initData )
    : _impl( new detail::DataSource( initData, _volumeInfo ) )
{
    if(!fillRegularVolumeInfo( _volumeInfo  ))
        LBTHROW( std::runtime_error( "Cannot setup the regular tree" ));
}

DataSource::~DataSource()
{
    
}

MemoryUnitPtr DataSource::getData( const LODNode& node )
{
    return _impl->sample( node );
}

bool DataSource::handles( const VolumeDataSourcePluginData& initData )
{
    const std::string remote = "remote";
    const std::string& scheme = initData.getURI().getScheme();
    return scheme.substr(0, remote.size()) == remote;
}

}
}
