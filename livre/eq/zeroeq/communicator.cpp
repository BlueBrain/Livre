
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

#include <livre/core/data/Histogram.h>

#include <lexis/lexis.h>
#include <zeroeq/zeroeq.h>
#include <lunchbox/clock.h>

#include <functional>
#include <map>
#include <unordered_map>

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

    bool publishFrame()
    {
        const auto& frameSettings = _getFrameData().getFrameSettings();
        const auto& params = _config.getApplicationParameters();

        _frame.setStart( params.frames[0] );
        _frame.setCurrent( frameSettings.getFrameNumber( ));
        _frame.setEnd( params.frames[1] );
        _frame.setDelta( params.animation );

        return _publisher.publish( _frame );
    }

    bool publishCamera()
    {
        return _publisher.publish( _getFrameData().getCameraSettings( ));
    }

    bool publishHistogram( const Histogram& histogram )
    {
        return _publisher.publish( histogram );
    }

    bool frameDirty()
    {
        const auto& frameSettings = _getFrameData().getFrameSettings();
        const auto& params = _config.getApplicationParameters();
        return _frame.getCurrent() != frameSettings.getFrameNumber() ||
               _frame.getDelta() != params.animation ||
               _frame.getStart() != params.frames.x() ||
               _frame.getEnd() != params.frames.y();
    }

    bool onRequest( ::lexis::ConstRequestPtr request )
    {
        const auto& i = _requests.find( request->getEvent( ));
        if( i == _requests.end( ))
            return false;
        return i->second();
    }

    void updateFrame()
    {
        if( _config.getDataFrameCount() == 0 )
            return;

        auto& frameSettings = _config.getFrameData().getFrameSettings();
        auto& params = _config.getApplicationParameters();

        if( _frame.getCurrent() == frameSettings.getFrameNumber() &&
            _frame.getDelta() == params.animation &&
            _frame.getStart() == params.frames.x() &&
            _frame.getEnd() == params.frames.y( ))
        {
            return;
        }

        frameSettings.setFrameNumber( _frame.getCurrent() );
        params.animation = _frame.getDelta();
        params.frames = { _frame.getStart(), _frame.getEnd() };
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
#ifdef ZEROEQ_USE_HTTPXX
    std::unique_ptr< ::zeroeq::http::Server > _httpServer;
#endif
    ::lexis::render::Frame _frame;
    Config& _config;

    void _setupRequests()
    {
        _requests[ _frame.getTypeIdentifier() ] = [&]
            { return publishFrame(); };
        _requests[ _getFrameData().getVRParameters().getTypeIdentifier( )] = [&]
            { return _publisher.publish( _getFrameData().getVRParameters( )); };
        _requests[ _getFrameData().getCameraSettings().getTypeIdentifier( )] = [&]
            { return _publisher.publish( _getFrameData().getCameraSettings());};
        _requests[ _getRenderSettings().getTransferFunction().getTypeIdentifier( )] = [&]
            {
                return _publisher.publish(
                    _getRenderSettings().getTransferFunction( ));
            };
    }

    void _setupHTTPServer( const int argc LB_UNUSED, char** argv LB_UNUSED )
    {
#ifdef ZEROEQ_USE_HTTPXX
        _httpServer = ::zeroeq::http::Server::parse( argc, argv, _subscriber );

        if( !_httpServer )
            return;

        _httpServer->subscribe( ::lexis::render::Exit::ZEROBUF_TYPE_NAME(),
                                [&] { return requestExit(); } );

        _httpServer->register_( ::lexis::render::ImageJPEG::ZEROBUF_TYPE_NAME(),
                                [&](){ return _config.renderJPEG(); });

        _httpServer->add( _frame );
        _httpServer->add( _getFrameData().getVRParameters( ));
        _httpServer->add( _getFrameData().getCameraSettings( ));
        _httpServer->add( _getRenderSettings().getTransferFunction( ));
#endif
    }

    void _setupSubscriber()
    {
        _subscriber.subscribe( ::lexis::Request::ZEROBUF_TYPE_IDENTIFIER(),
            [&]( const void* data, const size_t size )
            {
                onRequest( ::lexis::Request::create( data, size ));
            });

        _frame.registerDeserializedCallback( [&] { updateFrame(); });
        _subscriber.subscribe( _frame );

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

void Communicator::publishFrame()
{
    if( _impl->frameDirty( ))
        _impl->publishFrame();
}

bool Communicator::publishHistogram( const Histogram& histogram )
{
    return _impl->publishHistogram( histogram );
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
