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


#include "Controller.h"

#include <zeroeq/zeroeq.h>
#include <zerobuf/Zerobuf.h>

#include <algorithm>
#include <mutex>
#include <thread>

namespace livre
{

class Controller::Impl
{
public:
    Impl()
        : _subscriber()
        , _replySubscriber( (::zeroeq::Receiver&)_subscriber )
        , _subscriberPoll( std::bind( &Impl::pollSubscriber, this ))
        , _continuePolling( true )
    {
        _replySubscriber.registerHandler( ::zeroeq::vocabulary::EVENT_HEARTBEAT,
                     std::bind( &Impl::onHeartBeatReceived, this ));
    }

    ~Impl()
    {
        _replySubscriber.deregisterHandler( ::zeroeq::vocabulary::EVENT_HEARTBEAT );

        _continuePolling = false;
        _subscriberPoll.join();
    }

    void onHeartBeatReceived()
    {
        for( const auto& request : _requests )
            _publisher.publish( ::zeroeq::vocabulary::serializeRequest( request ));
    }

    void onReply( const ::zeroeq::Event& event )
    {
        const auto& i = std::find( _requests.begin(), _requests.end(),
                                   event.getType( ));
        if( i == _requests.end( ))
            return;

        _requests.erase( i );
        _replySubscriber.deregisterHandler( event.getType( ));
    }

    bool publish( const zeroeq::Event& event )
    {
        return _publisher.publish( event );
    }

    bool publish( const ::zerobuf::Zerobuf& zerobuf )
    {
        return _publisher.publish( zerobuf );
    }

    bool registerHandler( const zeroeq::uint128_t& event,
                          const zeroeq::EventFunc& func )
    {
        if( !_subscriber.registerHandler( event, func ))
            return false;

        _requests.push_back( event );
        return _replySubscriber.registerHandler( event,
                                                std::bind( &Impl::onReply, this,
                                                       std::placeholders::_1 ));
    }

    bool deregisterHandler( const zeroeq::uint128_t& event )
    {
        if( !_subscriber.deregisterHandler( event ))
            return false;

        const auto& i = std::find( _requests.begin(), _requests.end(), event );
        if( i != _requests.end( ))
        {
            _requests.erase( i );
            _replySubscriber.deregisterHandler( event );
        }

        return true;
    }

    bool subscribe( ::zerobuf::Zerobuf& zerobuf )
    {
        if( !_subscriber.subscribe( zerobuf ))
            return false;

        _requests.push_back( zerobuf.getTypeIdentifier( ));
        return _replySubscriber.registerHandler( zerobuf.getTypeIdentifier(),
                                                std::bind( &Impl::onReply, this,
                                                       std::placeholders::_1 ));
    }

    bool unsubscribe( const ::zerobuf::Zerobuf& zerobuf )
    {
        if( !_subscriber.unsubscribe( zerobuf ))
            return false;

        const auto& i = std::find( _requests.begin(), _requests.end(),
                                   zerobuf.getTypeIdentifier( ));
        if( i != _requests.end( ))
        {
            _requests.erase( i );
            _replySubscriber.deregisterHandler( zerobuf.getTypeIdentifier( ));
        }

        return true;
    }

private:
    void pollSubscriber()
    {
        while( _continuePolling )
            _subscriber.receive( 100 );
    }

    zeroeq::Publisher _publisher;
    zeroeq::Subscriber _subscriber;
    zeroeq::Subscriber _replySubscriber;

    std::thread _subscriberPoll;
    bool _continuePolling;

    std::vector< ::zeroeq::uint128_t > _requests;
};


Controller::Controller()
    : _impl( new Controller::Impl( ))
{}

Controller::~Controller()
{}

bool Controller::publish( const zeroeq::Event& event )
{
    return _impl->publish( event );
}

bool Controller::publish( const ::zerobuf::Zerobuf& zerobuf )
{
    return _impl->publish( zerobuf );
}

bool Controller::registerHandler( const zeroeq::uint128_t& event,
                                  const zeroeq::EventFunc& func )
{
    return _impl->registerHandler( event, func );
}

bool Controller::deregisterHandler( const zeroeq::uint128_t& event )
{
    return _impl->deregisterHandler( event );
}

bool Controller::subscribe( ::zerobuf::Zerobuf& zerobuf )
{
    return _impl->subscribe( zerobuf );
}

bool Controller::unsubscribe( const ::zerobuf::Zerobuf& zerobuf )
{
    return _impl->unsubscribe( zerobuf );
}

}
