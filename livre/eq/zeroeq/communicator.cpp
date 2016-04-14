
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

#include <lunchbox/clock.h>
#include <servus/uri.h>
#include <zeroeq/zeroeq.h>
#include <zeroeq/hbp/hbp.h>

#ifdef LIVRE_USE_RESTBRIDGE
#  include <restbridge/RestBridge.h>
#endif

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

        _setupPublisher();
        _setupRequests();
        _setupRESTBridge( argc, argv );
        _setupSubscribers();
    }

    void publishModelView( const Matrix4f& modelView )
    {
        if( !_publisher )
            return;

        const Floats matrix( modelView.data(), modelView.data() + 16 );
        _publisher->publish( ::zeroeq::hbp::serializeCamera( matrix ));
    }

    void publishCamera()
    {
        if( !_publisher )
            return;

        const auto& cameraSettings = _getFrameData().getCameraSettings();
        const Matrix4f& modelView = cameraSettings.getModelViewMatrix();
        const Floats matrix( modelView.data(), modelView.data() + 16 );
        _publisher->publish( ::zeroeq::hbp::serializeCamera( matrix ));
    }

    void publishExit()
    {
        if( !_publisher )
            return;

        _publisher->publish( ::zeroeq::Event( ::zeroeq::vocabulary::EVENT_EXIT ));
    }

    void publishLookupTable1D()
    {
        if( !_publisher )
            return;

        const auto& renderSettings = _getFrameData().getRenderSettings();
        const auto& lut = renderSettings.getTransferFunction().getData();
        _publisher->publish( ::zeroeq::hbp::serializeLookupTable1D( lut ) );
    }

    void publishFrame()
    {
        if( !_publisher )
            return;

        const auto& frameSettings = _getFrameData().getFrameSettings();
        const auto& params = _config.getApplicationParameters();

        const ::zeroeq::Event& frame = ::zeroeq::hbp::serializeFrame(
                                        ::zeroeq::hbp::data::Frame(
                                            params.frames[0],
                                            frameSettings.getFrameNumber(),
                                            params.frames[1],
                                            params.animation ));
        _publisher->publish( frame );
    }

    void publishVolumeRendererParameters()
    {
         _publisher->publish( _getFrameData().getVRParameters( ));
    }

    void publishVocabulary()
    {
        if( !_publisher )
            return;

        ::zeroeq::EventDescriptors vocabulary;
        vocabulary.push_back(
                    ::zeroeq::EventDescriptor( ::zeroeq::hbp::IMAGEJPEG,
                                               ::zeroeq::hbp::EVENT_IMAGEJPEG,
                                               ::zeroeq::hbp::SCHEMA_IMAGEJPEG,
                                               ::zeroeq::PUBLISHER ));
        vocabulary.push_back(
                    ::zeroeq::EventDescriptor( ::zeroeq::hbp::CAMERA,
                                               ::zeroeq::hbp::EVENT_CAMERA,
                                               ::zeroeq::hbp::SCHEMA_CAMERA,
                                               ::zeroeq::BIDIRECTIONAL ));
        vocabulary.push_back(
                    ::zeroeq::EventDescriptor( ::zeroeq::vocabulary::EXIT,
                                               ::zeroeq::vocabulary::EVENT_EXIT,
                                               ::zeroeq::vocabulary::SCHEMA_EXIT,
                                               ::zeroeq::PUBLISHER ));
        vocabulary.push_back(
                    ::zeroeq::EventDescriptor( ::zeroeq::hbp::LOOKUPTABLE1D,
                                               ::zeroeq::hbp::EVENT_LOOKUPTABLE1D,
                                               ::zeroeq::hbp::SCHEMA_LOOKUPTABLE1D,
                                               ::zeroeq::BIDIRECTIONAL ));
        vocabulary.push_back(
                    ::zeroeq::EventDescriptor( ::zeroeq::hbp::FRAME,
                                               ::zeroeq::hbp::EVENT_FRAME,
                                               ::zeroeq::hbp::SCHEMA_FRAME,
                                               ::zeroeq::BIDIRECTIONAL ));
        const auto& event = ::zeroeq::vocabulary::serializeVocabulary( vocabulary );
        _publisher->publish( event );
    }

    void publishHeartbeat()
    {
        if( !_publisher )
            return;

        if( _heartbeatClock.getTimef() >= DEFAULT_HEARTBEAT_TIME )
        {
            _heartbeatClock.reset();
            _publisher->publish(
                ::zeroeq::Event( ::zeroeq::vocabulary::EVENT_HEARTBEAT ));
        }
    }

    void publishImageJPEG( const uint8_t* data, const uint64_t size )
    {
        if( !_publisher )
            return;

        const ::zeroeq::hbp::data::ImageJPEG image( size, data );
        const auto& event = ::zeroeq::hbp::serializeImageJPEG( image );
        _publisher->publish( event );
    }

    void onRequest( const ::zeroeq::Event& event )
    {
        const auto& eventType = ::zeroeq::vocabulary::deserializeRequest( event );
        const auto& i = _requests.find( eventType );
        if( i != _requests.end( ))
            i->second();
    }

    // Generic camera (from REST) in meters
    void onCamera( const ::zeroeq::Event& event )
    {
        const auto& matrix = ::zeroeq::hbp::deserializeCamera( event );
        const Matrix4f modelViewMatrix( matrix );
        auto& cameraSettings = _getFrameData().getCameraSettings();
        cameraSettings.setModelViewMatrix( modelViewMatrix );
    }

    // HBP 'micron' camera from other brain applications
    void onHBPCamera( const ::zeroeq::Event& event )
    {
        const auto& matrix = ::zeroeq::hbp::deserializeCamera( event );
        const Matrix4f modelViewMatrixMicron( matrix );

        const auto& modelViewMatrix =
                _config.convertFromHBPCamera( modelViewMatrixMicron );
        auto& cameraSettings = _getFrameData().getCameraSettings();
        cameraSettings.setModelViewMatrix( modelViewMatrix );
    }

    void onLookupTable1D( const ::zeroeq::Event& event )
    {
        const TransferFunction1D transferFunction(
            ::zeroeq::hbp::deserializeLookupTable1D( event ));
        auto& renderSettings = _getRenderSettings();
        renderSettings.setTransferFunction( transferFunction );
    }

    void onFrame( const ::zeroeq::Event& event )
    {
        const auto& frame = ::zeroeq::hbp::deserializeFrame( event );

        if( _config.getDataFrameCount() == 0 )
            return;

        auto& frameSettings = _getFrameData().getFrameSettings();
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

    void requestImageJPEG()
    {
        _getFrameData().getFrameSettings().setGrabFrame( true );
    }

    void requestExit()
    {
        _config.stopRunning();
    }

    void handleEvents()
    {
        // Receiving all queued events from all receivers without blocking.
        for( auto subscriber : subscribers )
            while( subscriber->receive( 0 ))
                _config.sendEvent( REDRAW );
    }

private:
    void _setupPublisher()
    {
        _publisher.reset( new ::zeroeq::Publisher );
    }

    void _setupRequests()
    {
        _requests[::zeroeq::hbp::EVENT_CAMERA] =
            std::bind( &Impl::publishCamera, this );
        _requests[::zeroeq::hbp::EVENT_FRAME] =
            std::bind( &Impl::publishFrame, this );
        _requests[::zeroeq::hbp::EVENT_LOOKUPTABLE1D] =
            std::bind( &Impl::publishLookupTable1D, this );
        _requests[::zeroeq::hbp::EVENT_IMAGEJPEG] =
            std::bind( &Impl::requestImageJPEG, this );
        _requests[::zeroeq::vocabulary::EVENT_EXIT] =
            std::bind( &Impl::requestExit, this );
        const auto& renderParams = _getFrameData().getVRParameters();
        _requests[ renderParams.getTypeIdentifier( )] = [&]
            { _publisher->publish( _getFrameData().getVRParameters( )); };
    }

    void _setupRESTBridge( const int argc LB_UNUSED, char** argv LB_UNUSED )
    {
#ifdef LIVRE_USE_RESTBRIDGE
        _restBridge = restbridge::RestBridge::parse( *_publisher, argc, argv );
        if( !_restBridge )
            return;

        SubscriberPtr subscriber(
            new ::zeroeq::Subscriber( _restBridge->getSubscriberURI( )));
        subscribers.push_back( subscriber );
        subscriber->registerHandler( ::zeroeq::hbp::EVENT_CAMERA,
                                        std::bind( &Impl::onCamera, this,
                                                   std::placeholders::_1 ));
        subscriber->registerHandler( ::zeroeq::vocabulary::EVENT_REQUEST,
                                        std::bind( &Impl::onRequest, this,
                                                   std::placeholders::_1 ));
        subscriber->registerHandler( ::zeroeq::hbp::EVENT_FRAME,
                                        std::bind( &Impl::onFrame, this,
                                                   std::placeholders::_1 ));
        subscriber->registerHandler( ::zeroeq::hbp::EVENT_LOOKUPTABLE1D,
                                        std::bind( &Impl::onLookupTable1D, this,
                                                   std::placeholders::_1 ));
#endif
    }

    void _setupSubscribers()
    {
        SubscriberPtr subscriber( new ::zeroeq::Subscriber );

        subscribers.push_back( subscriber );
        subscriber->registerHandler( ::zeroeq::hbp::EVENT_CAMERA,
                                     std::bind( &Impl::onHBPCamera,
                                                this, std::placeholders::_1 ));
        subscriber->registerHandler( ::zeroeq::hbp::EVENT_LOOKUPTABLE1D,
                                     std::bind( &Impl::onLookupTable1D,
                                                this, std::placeholders::_1 ));
        subscriber->registerHandler( ::zeroeq::hbp::EVENT_FRAME,
                                     std::bind( &Impl::onFrame,
                                                 this, std::placeholders::_1 ));
        subscriber->registerHandler( ::zeroeq::vocabulary::EVENT_REQUEST,
                                     std::bind( &Impl::onRequest,
                                                this, std::placeholders::_1 ));
        subscriber->subscribe( _getFrameData().getVRParameters( ));
    }

    FrameData& _getFrameData() { return _config.getFrameData(); }
    const FrameData& _getFrameData() const { return _config.getFrameData(); }
    RenderSettings& _getRenderSettings()
        { return _getFrameData().getRenderSettings(); }
    const RenderSettings& _getRenderSettings() const
        { return _getFrameData().getRenderSettings(); }

    typedef std::shared_ptr< ::zeroeq::Subscriber > SubscriberPtr;
    typedef std::shared_ptr< ::zeroeq::Publisher > PublisherPtr;
    typedef std::vector< SubscriberPtr > Subscribers;

    Subscribers subscribers;
    PublisherPtr _publisher;
    lunchbox::Clock _heartbeatClock;
    typedef std::function< void() > RequestFunc;
    typedef std::map< ::zeroeq::uint128_t, RequestFunc > RequestFuncs;
    RequestFuncs _requests;
#ifdef LIVRE_USE_RESTBRIDGE
    std::unique_ptr< restbridge::RestBridge > _restBridge;
#endif
    Config& _config;
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

void Communicator::publishModelView( const Matrix4f& modelView )
{
    _impl->publishModelView( modelView );
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

void Communicator::handleEvents()
{
    _impl->handleEvents();
}

}
}
