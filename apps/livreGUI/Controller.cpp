/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Maxim Makhinya <maxmah@gmail.com>
 *                          David Steiner  <steiner@ifi.uzh.ch>
 *                          Fatih Erol
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


#include <livreGUI/Controller.h>

#include <zeq/subscriber.h>
#include <zeq/publisher.h>
#include <zeq/event.h>

#include <zeq/hbp/vocabulary.h>

#ifdef LIVRE_USE_ISC
#   include <isc/steering/simulator.h>
#endif

#include <boost/thread.hpp>

namespace livre
{

struct Controller::Impl
{

    typedef std::map< std::string, zeq::Publisher* > PublisherMap;
    typedef std::pair< std::string, zeq::Subscriber* > URISubscriberPair;
    typedef std::map< std::string, zeq::Subscriber* > SubscriberMap;

#ifdef LIVRE_USE_ISC
    typedef std::map< std::string, isc::Simulator* > SimulatorMap;
#endif

    Impl()
        : _subscriberPoll( boost::bind( &Impl::pollSubscribers, this ))
        , _continuePolling( true )

    {}

    template< typename Type >
    void deleteObjects( const std::map< std::string, Type* >& map )
    {
        typedef std::pair< std::string, Type* > Pair;
        BOOST_FOREACH( const Pair& pair, map )
        {
            Type* object = pair.second;
            delete object;
        }
    }

    ~Impl()
    {
        _continuePolling = false;
        _subscriberPoll.join();
        deleteObjects< zeq::Subscriber >( _subscriberMap );
        deleteObjects< zeq::Publisher >( _publisherMap );
#ifdef LIVRE_USE_ISC
        deleteObjects< isc::Simulator >( _simulatorMap );
#endif
    }

    bool connect( const std::string&, const uint16_t )
    {
        LBUNIMPLEMENTED;
        return false;
    }

    std::string getURIStr( const servus::URI& uri ) const
    {
        std::stringstream ss;
        ss << uri;
        return ss.str();
    }

    template< typename Type >
    Type* getObject( std::map< std::string, Type* >& map,
                const servus::URI& uri  )
    {
        const std::string& uriStr = getURIStr( uri );
        typename std::map< std::string, Type* >::iterator it = map.find( uriStr );
        if( it == map.end( ))
            map[ uriStr ] = new Type( uri );

        return map[ uriStr ];
    }

    zeq::Subscriber* getSubscriber( const servus::URI& uri )
    {
        return getObject<zeq::Subscriber>( _subscriberMap, uri );
    }

    bool registerHandler( const servus::URI& uri,
                          const servus::uint128_t& event,
                          const zeq::EventFunc& func )
    {
        ScopedLock lock( _subscriberMutex );
        zeq::Subscriber* subscriber = getSubscriber( uri );
        return subscriber->registerHandler( event, func );
    }

    bool deregisterHandler( const servus::URI& uri,
                            const servus::uint128_t& event )
    {
        ScopedLock lock( _subscriberMutex );
        zeq::Subscriber* subscriber = getSubscriber( uri );
        return subscriber->deregisterHandler( event );
    }

    zeq::Publisher* getPublisher( const servus::URI& uri )
    {
        return getObject<zeq::Publisher>( _publisherMap, uri );
    }

    bool publish( const servus::URI& uri, const zeq::Event& event )
    {
        return getPublisher( uri )->publish( event );
    }

#ifdef LIVRE_USE_ISC
    isc::Simulator* getSimulator( const servus::URI& uri )
    {
        return getObject<isc::Simulator>( _simulatorMap, uri );
    }
#endif

    void pollSubscribers()
    {
        /* Polling can be done through the QTimer object ( will simplify
           locking issues ), which is part of the main loop, but this class
           has no Qt dependency, currently we are using regular threads.
        */
        while( _continuePolling )
        {
            ScopedLock lock( _subscriberMutex );
            BOOST_FOREACH( const URISubscriberPair& pair, _subscriberMap )
            {
                zeq::Subscriber* subscriber = pair.second;
                subscriber->receive( 100 );
            }
        }
    }

    PublisherMap _publisherMap;
    SubscriberMap _subscriberMap;

#ifdef LIVRE_USE_ISC
    SimulatorMap _simulatorMap;
#endif

    boost::mutex _subscriberMutex;
    boost::thread _subscriberPoll;
    bool _continuePolling;
};


Controller::Controller( )
    : _impl( new Controller::Impl( ))
{}

Controller::~Controller( )
{}

bool Controller::connect( const std::string& hostname,
                          const uint16_t port )
{
    return _impl->connect( hostname, port );
}

bool Controller::publish( const servus::URI& uri,
                          const zeq::Event& event )
{
    return _impl->publish( uri, event );
}

bool Controller::registerHandler( const servus::URI& uri,
                                  const uint128_t& event,
                                  const zeq::EventFunc& func )
{
     return _impl->registerHandler( uri, event, func );
}

bool Controller::deregisterHandler( const servus::URI& uri,
                                    const servus::uint128_t& event )
{
    return _impl->deregisterHandler( uri, event );
}

#ifdef LIVRE_USE_ISC
isc::Simulator* Controller::getSimulator( const servus::URI& uri )
{
    return _impl->getSimulator( uri );
}
#endif

}
