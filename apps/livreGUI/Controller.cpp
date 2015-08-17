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
        : _transferFunction( new TransferFunction1Df())
        , _subscriberPoll( boost::bind( &Impl::pollSubscribers, this ))
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

    zeq::Publisher* getPublisher( const servus::URI& uri )
    {
        return getObject<zeq::Publisher>( _publisherMap, uri );
    }

    zeq::Subscriber* getSubscriber( const servus::URI& uri )
    {
        ScopedLock lock( _subscriberMutex );
        return getObject<zeq::Subscriber>( _subscriberMap, uri );
    }

#ifdef LIVRE_USE_ISC
    isc::Simulator* getSimulator( const servus::URI& uri )
    {
        return getObject<isc::Simulator>( _simulatorMap, uri );
    }
#endif

    void pollSubscribers()
    {
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

    void publishTransferFunction( )
    {
        UInt8Vector rgbai;
        const FloatVector& rgbaf = _transferFunction->getData();
        rgbai.resize( rgbaf.size(), 0 );

        for( uint32_t i = 0; i < rgbaf.size( ); ++i )
        {
            rgbai[ i ] = uint8_t( rgbaf[ i ] *
                                  std::numeric_limits< uint8_t >::max( ));
        }
        zeq::Publisher *publisher = getPublisher( servus::URI( "hbp://" ));
        publisher->publish( zeq::hbp::serializeLookupTable1D( rgbai ));
    }

    PublisherMap _publisherMap;
    SubscriberMap _subscriberMap;

#ifdef LIVRE_USE_ISC
    SimulatorMap _simulatorMap;
#endif

    TransferFunction1DfPtr _transferFunction;

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

zeq::Publisher* Controller::getPublisher( const servus::URI& uri )
{
    return _impl->getPublisher( uri );
}

zeq::Subscriber* Controller::getSubscriber( const servus::URI& uri )
{
    return _impl->getSubscriber( uri );
}

#ifdef LIVRE_USE_ISC
isc::Simulator* Controller::getSimulator( const servus::URI& uri )
{
    return _impl->getSimulator( uri );
}
#endif

void Controller::publishTransferFunction( )
{
    _impl->publishTransferFunction();
}

TransferFunction1DfPtr Controller::getTransferFunction() const
{
    return _impl->_transferFunction;
}

}
