/* Copyright (c) 2006-2015, Daniel.Nachbaur@epfl.ch
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

#include <lunchbox/clock.h>
#include <servus/uri.h>
#include <zeq/zeq.h>
#include <zeq/hbp/hbp.h>

#ifdef LIVRE_USE_RESTBRIDGE
#  include <restbridge/RestBridge.h>
#endif

#include <functional>
#include <map>
#include <unordered_map>

#define DEFAULT_HEARTBEAT_TIME 1000.0f

namespace livre
{
namespace zeq
{
class Communicator::Impl
{
public:
    Impl( Config& config, const int argc, char** argv )
        : _vwsPublisher()
        , _config( config )
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

        const FloatVector matrix( modelView.begin(), modelView.end( ));
        _publisher->publish( ::zeq::hbp::serializeCamera( matrix ));
    }

    void publishCamera()
    {
        if( !_publisher )
            return;

        const auto cameraSettings = _config.getFrameData().getCameraSettings();
        const Matrix4f& modelView = cameraSettings->getModelViewMatrix();
        const FloatVector matrix( modelView.begin(), modelView.end( ));
        _vwsPublisher->publish( ::zeq::hbp::serializeCamera( matrix ));
    }

    void publishExit()
    {
        if( !_publisher )
            return;

        _publisher->publish( ::zeq::Event( ::zeq::vocabulary::EVENT_EXIT ));
        _vwsPublisher->publish( ::zeq::Event( ::zeq::vocabulary::EVENT_EXIT ));
    }

    void publishLookupTable1D()
    {
        if( !_publisher )
            return;

        const auto& renderSettings = _config.getFrameData().getRenderSettings();
        const auto& lut = renderSettings->getTransferFunction().getData();
        _vwsPublisher->publish( ::zeq::hbp::serializeLookupTable1D( lut ));
        _publisher->publish( ::zeq::hbp::serializeLookupTable1D( lut ) );
    }

    void publishFrame()
    {
        if( !_publisher )
            return;

        const auto& frameSettings = _config.getFrameData().getFrameSettings();
        const auto& params = _config.getApplicationParameters();

        const Vector2ui& dataFrameRange = _config.getDataFrameRange();

        const uint32_t frameMin =
                std::max( params.frames.x(), dataFrameRange[ 0 ]);

        const uint32_t frameMax =
                std::min( params.frames.y(), dataFrameRange[ 1 ]);

        const ::zeq::Event& frame = ::zeq::hbp::serializeFrame(
                                        ::zeq::hbp::data::Frame(
                                            frameMin,
                                            frameSettings->getFrameNumber(),
                                            frameMax,
                                            params.animation ));
        _publisher->publish( frame );
        _vwsPublisher->publish( frame );
    }

    void publishVocabulary()
    {
        if( !_vwsPublisher )
            return;

        ::zeq::EventDescriptors vocabulary;
        vocabulary.push_back(
                    ::zeq::EventDescriptor( ::zeq::hbp::IMAGEJPEG,
                                            ::zeq::hbp::EVENT_IMAGEJPEG,
                                            ::zeq::hbp::SCHEMA_IMAGEJPEG,
                                            ::zeq::PUBLISHER ));
        vocabulary.push_back(
                    ::zeq::EventDescriptor( ::zeq::hbp::CAMERA,
                                            ::zeq::hbp::EVENT_CAMERA,
                                            ::zeq::hbp::SCHEMA_CAMERA,
                                            ::zeq::BIDIRECTIONAL ));
        vocabulary.push_back(
                    ::zeq::EventDescriptor( ::zeq::vocabulary::EXIT,
                                            ::zeq::vocabulary::EVENT_EXIT,
                                            ::zeq::vocabulary::SCHEMA_EXIT,
                                            ::zeq::PUBLISHER ));
        vocabulary.push_back(
                    ::zeq::EventDescriptor( ::zeq::hbp::LOOKUPTABLE1D,
                                            ::zeq::hbp::EVENT_LOOKUPTABLE1D,
                                            ::zeq::hbp::SCHEMA_LOOKUPTABLE1D,
                                            ::zeq::BIDIRECTIONAL ));
        vocabulary.push_back(
                    ::zeq::EventDescriptor( ::zeq::hbp::FRAME,
                                            ::zeq::hbp::EVENT_FRAME,
                                            ::zeq::hbp::SCHEMA_FRAME,
                                            ::zeq::BIDIRECTIONAL ));
        const auto& event = ::zeq::vocabulary::serializeVocabulary( vocabulary);
        _vwsPublisher->publish( event );
    }

    void publishHeartbeat()
    {
        if( !_publisher )
            return;

        if( _heartbeatClock.getTimef() >= DEFAULT_HEARTBEAT_TIME )
        {
            _heartbeatClock.reset();
            _vwsPublisher->publish(
                ::zeq::Event( ::zeq::vocabulary::EVENT_HEARTBEAT ));
            _publisher->publish(
                ::zeq::Event( ::zeq::vocabulary::EVENT_HEARTBEAT ));
        }
    }

    void publishImageJPEG( const uint8_t* data, const uint64_t size )
    {
        if( !_vwsPublisher )
            return;

        const ::zeq::hbp::data::ImageJPEG image( size, data );
        const auto& event = ::zeq::hbp::serializeImageJPEG( image );
        _vwsPublisher->publish( event );
    }

    void onRequest( const ::zeq::Event& event )
    {
        const auto& eventType = ::zeq::vocabulary::deserializeRequest( event );
        const auto& i = _requests.find( eventType );
        if( i != _requests.end( ))
            i->second();
    }

    // Generic camera (from REST) in meters
    void onCamera( const ::zeq::Event& event )
    {
        const auto& matrix = ::zeq::hbp::deserializeCamera( event );
        Matrix4f modelViewMatrix;
        modelViewMatrix.set( matrix.begin(), matrix.end(), false );
        auto cameraSettings = _config.getFrameData().getCameraSettings();
        cameraSettings->setModelViewMatrix( modelViewMatrix );
        _config.sendEvent( REDRAW );
    }

    // HBP 'micron' camera from other brain applications
    void onHBPCamera( const ::zeq::Event& event )
    {
        const auto& matrix = ::zeq::hbp::deserializeCamera( event );
        Matrix4f modelViewMatrixMicron;
        modelViewMatrixMicron.set( matrix.begin(), matrix.end(), false );

        const auto& modelViewMatrix =
                _config.convertFromHBPCamera( modelViewMatrixMicron );
        auto cameraSettings = _config.getFrameData().getCameraSettings();
        cameraSettings->setModelViewMatrix( modelViewMatrix );
        _config.sendEvent( REDRAW );
    }

    void onLookupTable1D( const ::zeq::Event& event )
    {
        const TransferFunction1Dc transferFunction(
            ::zeq::hbp::deserializeLookupTable1D( event ));
        auto renderSettings = _config.getFrameData().getRenderSettings();
        renderSettings->setTransferFunction( transferFunction );
        _config.sendEvent( REDRAW );
    }

    void onFrame( const ::zeq::Event& event )
    {
        const auto& frame = ::zeq::hbp::deserializeFrame( event );

        if( _config.getDataFrameCount() == 0 )
            return;

        auto frameSettings = _config.getFrameData().getFrameSettings();
        auto& params = _config.getApplicationParameters();

        if( frame.current == frameSettings->getFrameNumber() &&
            frame.delta == params.animation )
        {
            return;
        }

        frameSettings->setFrameNumber( frame.current );
        params.animation = frame.delta;
        _config.sendEvent( REDRAW );
    }

    void requestImageJPEG()
    {
        _config.getFrameData().getFrameSettings()->setGrabFrame( true );
        _config.sendEvent( REDRAW );
    }

    void requestExit()
    {
        _config.stopRunning();
        _config.sendEvent( REDRAW );
    }

    void handleEvents()
    {
        // Receiving all queued events from all receivers without blocking.
        for( auto subscriber : subscribers )
            while( subscriber->receive( 0 ))
                /*nop*/ ;
    }

private:
    void _setupPublisher()
    {
        const auto& params = _config.getApplicationParameters();
        const servus::URI zeqSchema( params.zeqSchema );
        _publisher.reset( new ::zeq::Publisher( zeqSchema ));
    }

    void _setupRequests()
    {
        _requests[::zeq::hbp::EVENT_CAMERA] =
                std::bind( &Impl::publishCamera, this );
        _requests[::zeq::hbp::EVENT_FRAME] =
                std::bind( &Impl::publishFrame, this );
        _requests[::zeq::hbp::EVENT_LOOKUPTABLE1D] =
                std::bind( &Impl::publishLookupTable1D, this );
        _requests[::zeq::hbp::EVENT_IMAGEJPEG] =
                std::bind( &Impl::requestImageJPEG, this );
        _requests[::zeq::vocabulary::EVENT_EXIT] =
                std::bind( &Impl::requestExit, this );
        _requests[::zeq::vocabulary::EVENT_VOCABULARY] =
                std::bind( &Impl::publishVocabulary, this );
    }

    void _setupRESTBridge( const int argc, char** argv )
    {
#ifdef LIVRE_USE_RESTBRIDGE
        _restBridge = restbridge::RestBridge::parse( argc, argv );
        if( !_restBridge )
            return;

        _vwsPublisher.reset( new ::zeq::Publisher( _restBridge->getPublisherURI()));
        SubscriberPtr subscriber(
            new ::zeq::Subscriber( _restBridge->getSubscriberURI( )));
        subscribers.push_back( subscriber );
        subscriber->registerHandler( ::zeq::hbp::EVENT_CAMERA,
                                        std::bind( &Impl::onCamera, this,
                                                   std::placeholders::_1 ));
        subscriber->registerHandler( ::zeq::vocabulary::EVENT_REQUEST,
                                        std::bind( &Impl::onRequest, this,
                                                   std::placeholders::_1 ));
        subscriber->registerHandler( ::zeq::hbp::EVENT_FRAME,
                                        std::bind( &Impl::onFrame, this,
                                                   std::placeholders::_1 ));
        subscriber->registerHandler( ::zeq::hbp::EVENT_LOOKUPTABLE1D,
                                        std::bind( &Impl::onLookupTable1D, this,
                                                   std::placeholders::_1 ));
#else
        _vwsPublisher.reset( new ::zeq::Publisher( servus::URI( "vwsrep://" )));
#endif
    }

    void _setupSubscribers()
    {
        SubscriberPtr subscriber(
                    new ::zeq::Subscriber(
                        servus::URI( _config.getApplicationParameters().zeqSchema )));

        subscribers.push_back( subscriber );
        if( _config.getApplicationParameters().syncCamera )
        {
            subscriber->registerHandler( ::zeq::hbp::EVENT_CAMERA,
                                         std::bind( &Impl::onHBPCamera,
                                                 this, std::placeholders::_1 ));
        }
        subscriber->registerHandler( ::zeq::hbp::EVENT_LOOKUPTABLE1D,
                                     std::bind( &Impl::onLookupTable1D,
                                                this, std::placeholders::_1 ));
        subscriber->registerHandler( ::zeq::hbp::EVENT_FRAME,
                                     std::bind( &Impl::onFrame,
                                                 this, std::placeholders::_1 ));
        subscriber->registerHandler( ::zeq::vocabulary::EVENT_REQUEST,
                                     std::bind( &Impl::onRequest,
                                                this, std::placeholders::_1 ));
    }

    typedef std::shared_ptr< ::zeq::Subscriber > SubscriberPtr;
    typedef std::shared_ptr< ::zeq::Publisher > PublisherPtr;
    typedef std::vector< SubscriberPtr > Subscribers;

    Subscribers subscribers;
    PublisherPtr _publisher;
    lunchbox::Clock _heartbeatClock;
    PublisherPtr _vwsPublisher;
    typedef std::function< void() > RequestFunc;
    typedef std::map< ::zeq::uint128_t, RequestFunc > RequestFuncs;
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
