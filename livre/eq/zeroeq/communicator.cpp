
/* Copyright (c) 2015-2016, Daniel.Nachbaur@epfl.ch
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

#include "communicator.h"

#include <livre/eq/Config.h>
#include <livre/eq/Event.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/settings/CameraSettings.h>
#include <livre/eq/settings/FrameSettings.h>
#include <livre/eq/settings/RenderSettings.h>

#include <livre/lib/configuration/ApplicationParameters.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>

#include <zerobuf/render/lookupTable1D.h>
#include <zeroeq/zeroeq.h>
#include <zeroeq/hbp/hbp.h>
#include <lunchbox/clock.h>
#include <servus/uri.h>

#include <functional>
#include <map>
#include <unordered_map>

#define DEFAULT_HEARTBEAT_TIME 1000.0f

namespace livre
{
namespace zeroeq
{
class Communicator::Impl
{
public:
    Impl( Config& config, const int argc, char** argv )
        : _config( config )
    {
        if( !servus::Servus::isAvailable( ))
            return;

        _setupRequests();
        _setupSubscriber();
        _setupHTTPServer( argc, argv );
    }

    bool publishExit()
    {
        return _publisher.publish( ::zeroeq::Event( ::zeroeq::vocabulary::EVENT_EXIT ));
    }

    bool publishFrame()
    {
        const auto& frameSettings = _getFrameData().getFrameSettings();
        const auto& params = _config.getApplicationParameters();

        const ::zeroeq::Event& frame = ::zeroeq::hbp::serializeFrame(
                                        ::zeroeq::hbp::data::Frame(
                                            params.frames[0],
                                            frameSettings.getFrameNumber(),
                                            params.frames[1],
                                            params.animation ));
        return _publisher.publish( frame );
    }

    bool publishCamera()
    {
        return _publisher.publish( _getFrameData().getCameraSettings( ));
    }

    void publishHeartbeat()
    {
        if( _heartbeatClock.getTimef() >= DEFAULT_HEARTBEAT_TIME )
        {
            _heartbeatClock.reset();
            _publisher.publish(
                ::zeroeq::Event( ::zeroeq::vocabulary::EVENT_HEARTBEAT ));
        }
    }

    bool publishImageJPEG( const uint8_t* data, const uint64_t size )
    {
        const ::zeroeq::hbp::data::ImageJPEG image( size, data );
        const auto& event = ::zeroeq::hbp::serializeImageJPEG( image );
        return _publisher.publish( event );
    }

    bool onRequest( const ::zeroeq::Event& event )
    {
        const auto& eventType = ::zeroeq::vocabulary::deserializeRequest( event );
        const auto& i = _requests.find( eventType );
        if( i == _requests.end( ))
            return false;
        return i->second();
    }

    void onFrame( const ::zeroeq::Event& event )
    {
        const auto& frame = ::zeroeq::hbp::deserializeFrame( event );

        if( _config.getDataFrameCount() == 0 )
            return;

        auto& frameSettings = _config.getFrameData().getFrameSettings();
        auto& params = _config.getApplicationParameters();

        if( frame.current == frameSettings.getFrameNumber() &&
            frame.delta == params.animation &&
            frame.start == params.frames.x() &&
            frame.end == params.frames.y( ))
        {
            return;
        }

        frameSettings.setFrameNumber( frame.current );
        params.animation = frame.delta;
        params.frames = { frame.start, frame.end };
    }

    bool requestImageJPEG()
    {
        _config.getFrameData().getFrameSettings().setGrabFrame( true );
        return true;
    }

    bool requestExit()
    {
        _config.stopRunning();
        return true;
    }

    void handleEvents()
    {
        while( _subscriber.receive( 0 ))
            _config.sendEvent( REDRAW );
    }

private:
    ::zeroeq::Subscriber _subscriber;
    ::zeroeq::Publisher _publisher;
    lunchbox::Clock _heartbeatClock;
    typedef std::function< bool() > RequestFunc;
    typedef std::map< ::zeroeq::uint128_t, RequestFunc > RequestFuncs;
    RequestFuncs _requests;
#ifdef ZEQ_USE_HTTPXX
    std::unique_ptr< ::zeroeq::http::Server > _httpServer;
#endif
    Config& _config;

    void _setupRequests()
    {
        _requests[::zeroeq::hbp::EVENT_FRAME] = [&]{ return publishFrame(); };
        _requests[::zeroeq::hbp::EVENT_IMAGEJPEG] = [&]
            { return requestImageJPEG(); };
        _requests[::zeroeq::vocabulary::EVENT_EXIT] = [&]{ return requestExit(); };
        _requests[ VolumeRendererParameters::TYPE_IDENTIFIER() ] = [&]
            { return _publisher.publish( _getFrameData().getVRParameters( )); };
        _requests[ ::zerobuf::render::Camera::TYPE_IDENTIFIER( )] = [&]
            { return _publisher.publish( _getFrameData().getCameraSettings());};
        _requests[ ::zerobuf::render::LookupTable1D::TYPE_IDENTIFIER( )] = [&]
            {
                return _publisher.publish(
                    _getRenderSettings().getTransferFunction( ));
            };
    }

    void _setupHTTPServer( const int argc LB_UNUSED, char** argv LB_UNUSED )
    {
#ifdef ZEQ_USE_HTTPXX
        _httpServer = ::zeroeq::http::Server::parse( argc, argv, _subscriber );
        if( !_httpServer )
            return;

        // subscriber->registerHandler( ::zeroeq::hbp::EVENT_FRAME,
        //                                 std::bind( &Impl::onFrame, this,
        //                                            std::placeholders::_1 ));
        _httpServer->add( _getFrameData().getCameraSettings( ));
        _httpServer->add( _getRenderSettings().getTransferFunction( ));
#endif
    }

    void _setupSubscriber()
    {
        _subscriber.registerHandler( ::zeroeq::hbp::EVENT_FRAME,
                                     [ this ]( const ::zeroeq::Event& event )
                                         { onFrame( event ); } );
        _subscriber.registerHandler( ::zeroeq::vocabulary::EVENT_REQUEST,
                                     [ this ]( const ::zeroeq::Event& event )
                                         { onRequest( event ); } );
        _subscriber.subscribe( _getFrameData().getVRParameters( ));
        _subscriber.subscribe( _getFrameData().getCameraSettings( ));
        _subscriber.subscribe( _getRenderSettings().getTransferFunction( ));
    }

    FrameData& _getFrameData() { return _config.getFrameData(); }
    const FrameData& _getFrameData() const { return _config.getFrameData(); }
    RenderSettings& _getRenderSettings()
        { return _getFrameData().getRenderSettings(); }
    const RenderSettings& _getRenderSettings() const
        { return _getFrameData().getRenderSettings(); }
};

Communicator::Communicator( Config& config, const int argc, char* argv[] )
    : _impl( new Impl( config, argc, argv ))
{
}

Communicator::~Communicator()
{
}

void Communicator::publishImageJPEG( const uint8_t* data, const uint64_t size )
{
    _impl->publishImageJPEG( data, size );
}

void Communicator::publishHeartbeat()
{
    _impl->publishHeartbeat();
}

void Communicator::publishExit()
{
    _impl->publishExit();
}

void Communicator::publishFrame()
{
    _impl->publishFrame();
}

void Communicator::publishCamera()
{
    _impl->publishCamera();
}

void Communicator::handleEvents()
{
    _impl->handleEvents();
}

}
}
