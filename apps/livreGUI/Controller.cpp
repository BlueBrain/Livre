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

#include <boost/thread.hpp>

namespace livre
{

bool operator==( const zeq::EventFunc& func1, const zeq::EventFunc& func2 )
{
    return func1.target<void*>() == func2.target<void*>()
            && func1.target_type() == func2.target_type();
}

class Controller::Impl
{
public:
    Impl()
        : _currentSubscriber( 0 )
        , _subscriberPoll( boost::bind( &Impl::pollSubscribers, this ))
        , _continuePolling( true )
    {}

    ~Impl()
    {
        _continuePolling = false;
        _subscriberPoll.join();
        deleteObjects< zeq::Subscriber >( _subscriberMap );
        deleteObjects< zeq::Publisher >( _publisherMap );
    }

    bool connect( const std::string&, const uint16_t )
    {
        LBUNIMPLEMENTED;
        return false;
    }

    bool publish( const servus::URI& uri, const zeq::Event& event )
    {
        return getPublisher( uri )->publish( event );
    }

    bool registerHandler( const servus::URI& uri,
                          const servus::uint128_t& event,
                          const zeq::EventFunc& func )
    {
        ScopedLock lock( _subscriberMutex );
        return createSubscriberAndRegisterEvent( uri, event, func );
    }

    bool deregisterHandler( const servus::URI& uri,
                            const servus::uint128_t& event,
                            const zeq::EventFunc& func )
    {
        ScopedLock lock( _subscriberMutex );
        const std::string& uriStr = std::to_string( uri );
        if( _subscriberMap.count( uriStr ) == 0 )
            return false;

        zeq::Subscriber* subscriber = _subscriberMap[ uriStr ];

        SubscriberEventFunctionsList::iterator it =
                _subscriberEventFunctionList.begin();
        const EventIdEventFuncPair idFuncPair( event, func );
        BOOST_FOREACH( SubscriberEventFunctionsPair& pair,
                       _subscriberEventFunctionList )
        {
            const zeq::Subscriber* sub = pair.first;
            if( sub == subscriber )
            {
                const EventIdEventFuncPair& eventIdPair = pair.second;
                if( eventIdPair == idFuncPair )
                    break;
            }
            ++it;
        }

        if( it != _subscriberEventFunctionList.end( ))
            _subscriberEventFunctionList.erase( it );

        bool eventFound = false;
        BOOST_FOREACH( SubscriberEventFunctionsPair& pair,
                       _subscriberEventFunctionList )
        {
            const EventIdEventFuncPair& eventIdPair = pair.second;
            if( eventIdPair.eventId == event )
            {
                eventFound = true;
                break;
            }
        }

        if( !eventFound )
            subscriber->deregisterHandler( event );

        return false;
    }

private:
    struct EventIdEventFuncPair
    {
        EventIdEventFuncPair( const servus::uint128_t& eventId_,
                              const zeq::EventFunc eventFunc_ )
            : eventId( eventId_ ),
              eventFunc( eventFunc_ )
        {}

        bool operator==( const EventIdEventFuncPair& pair ) const
        {
            return eventId == pair.eventId &&
                    eventFunc == pair.eventFunc;
        }

        servus::uint128_t eventId;
        zeq::EventFunc eventFunc;
    };

    typedef std::map< std::string, zeq::Publisher* > PublisherMap;
    typedef std::pair< std::string, zeq::Subscriber* > URISubscriberPair;
    typedef std::map< std::string, zeq::Subscriber* > SubscriberMap;
    typedef std::pair< const zeq::Subscriber*, EventIdEventFuncPair >
                                                    SubscriberEventFunctionsPair;
    typedef std::list< SubscriberEventFunctionsPair >
                                                SubscriberEventFunctionsList;

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

    template< typename Type >
    Type* getObject( std::map< std::string, Type* >& map,
                     const servus::URI& uri )
    {
        const std::string& uriStr = std::to_string( uri );
        typename std::map< std::string, Type* >::iterator it = map.find( uriStr );
        if( it == map.end( ))
            map[ uriStr ] = new Type( uri );

        return map[ uriStr ];
    }

    void subscriberMultiplexer( const zeq::Event& event )
    {
        BOOST_FOREACH( const SubscriberEventFunctionsPair& pair,
                       _subscriberEventFunctionList )
        {
            const zeq::Subscriber* subscriber = pair.first;
            if( _currentSubscriber == subscriber )
            {
                const EventIdEventFuncPair& eventIdPair = pair.second;
                if( eventIdPair.eventId == event.getType( ))
                    eventIdPair.eventFunc( event );
            }
        }
    }

    bool createSubscriberAndRegisterEvent( const servus::URI& uri,
                                           const servus::uint128_t& event,
                                           const zeq::EventFunc& func )
    {
        const std::string& uriStr = std::to_string( uri );
        typename std::map< std::string, zeq::Subscriber* >::iterator it =
                _subscriberMap.find( uriStr );
        if( it == _subscriberMap.end( ))
        {
            zeq::Subscriber* subscriber = new zeq::Subscriber( uri );
            _subscriberMap[ uriStr ] = subscriber;
        }

        zeq::Subscriber* subscriber = _subscriberMap[ uriStr ];
        BOOST_FOREACH( SubscriberEventFunctionsPair& pair,
                       _subscriberEventFunctionList )
        {
            const zeq::Subscriber* sub = pair.first;
            if( sub == subscriber )
            {
                const EventIdEventFuncPair& eventIdPair = pair.second;
                if( eventIdPair.eventId == event )
                    return false;
            }
        }

        EventIdEventFuncPair eventIdPair( event, func );
        _subscriberEventFunctionList.push_back(
                    std::make_pair( subscriber, eventIdPair ));
        subscriber->registerHandler( event,
                                     boost::bind( &Impl::subscriberMultiplexer,
                                                  this, _1 ));
        return true;
    }

    void pollSubscribers()
    {
        while( _continuePolling )
        {
            SubscriberMap subscribers;
            {
                ScopedLock lock( _subscriberMutex );
                subscribers = _subscriberMap;
            }
            BOOST_FOREACH( const URISubscriberPair& pair, subscribers )
            {
                zeq::Subscriber* subscriber = pair.second;
                _currentSubscriber = subscriber;
                subscriber->receive( 100 );
            }
        }
    }

    zeq::Publisher* getPublisher( const servus::URI& uri )
    {
        return getObject<zeq::Publisher>( _publisherMap, uri );
    }

    PublisherMap _publisherMap;
    SubscriberMap _subscriberMap;
    SubscriberEventFunctionsList _subscriberEventFunctionList;
    zeq::Subscriber* _currentSubscriber;

    boost::mutex _subscriberMutex;
    boost::thread _subscriberPoll;
    bool _continuePolling;
};


Controller::Controller()
    : _impl( new Controller::Impl( ))
{}

Controller::~Controller()
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
                                  const servus::uint128_t& event,
                                  const zeq::EventFunc& func )
{
     return _impl->registerHandler( uri, event, func );
}

bool Controller::deregisterHandler( const servus::URI& uri,
                                    const servus::uint128_t& event,
                                    const zeq::EventFunc& func )
{
     return _impl->deregisterHandler( uri, event, func );
}
}
