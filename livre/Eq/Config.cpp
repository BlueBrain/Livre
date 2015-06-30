/* Copyright (c) 2006-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          David Steiner   <steiner@ifi.uzh.ch>
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

#include <livre/Eq/Config.h>

#include <livre/Eq/Event.h>
#include <livre/Eq/Events/EqEventHandler.h>
#include <livre/Eq/Events/EqEventHandlerFactory.h>
#include <livre/Eq/Events/EqEventInfo.h>
#include <livre/Eq/Events/Events.h>
#include <livre/Eq/FrameData.h>
#include <livre/Eq/Settings/CameraSettings.h>
#include <livre/Eq/Settings/FrameSettings.h>
#include <livre/Eq/Settings/RenderSettings.h>
#include <livre/Eq/Settings/VolumeSettings.h>
#include <livre/Eq/Window.h>
#include <livre/core/Events/EventMapper.h>
#include <livre/core/Maths/Maths.h>

#ifdef LIVRE_USE_ZEQ
#  include <zeq/zeq.h>
#  include <zeq/hbp/hbp.h>
#endif

#ifdef LIVRE_USE_RESTBRIDGE
#  include <restbridge/RestBridge.h>
static const std::string PUBLISHER_SCHEMA_SUFFIX = "resp://";
static const std::string SUBSCRIBER_SCHEMA_SUFFIX = "cmd://";
#endif

#include <eq/eq.h>
#include <lunchbox/uri.h>
#include <boost/bind.hpp>

#define DEFAULT_HEARTBEAT_TIME 1000.0f

namespace livre
{
namespace
{
#ifdef LIVRE_USE_ZEQ
typedef boost::shared_ptr< zeq::Subscriber > SubscriberPtr;
typedef boost::shared_ptr< zeq::Publisher > PublisherPtr;
typedef std::vector< SubscriberPtr > Subscribers;
#endif
}

namespace detail
{
class Config
{
public:
    Config( livre::Config* config_ )
        : config( config_ )
        , currentCanvas( 0 )
        , eventMapper( EventHandlerFactoryPtr( new EqEventHandlerFactory ))
        , volumeBBox( Boxf::makeUnitBox( ))
#ifdef LIVRE_USE_ZEQ
        , _publisher( lunchbox::URI( "hbp://" ))
        , _vwsPublisher()
#endif
    {}

    livre::Config* config;
    eq::Canvas* currentCanvas;
    EventMapper eventMapper;
    FrameData framedata;
    Boxf volumeBBox;

#ifdef LIVRE_USE_ZEQ
    void publishModelView()
    {
        CameraSettingsPtr cameraSettings = framedata.getCameraSettings();
        Matrix4f modelView = cameraSettings->getModelRotation();
        modelView.set_translation( cameraSettings->getCameraPosition() );
        modelView = cameraSettings->getCameraRotation() * modelView;

        Matrix3f rotation;
        Vector3f eyePos;
        maths::getRotationAndEyePositionFromModelView( modelView,
                                                       rotation,
                                                       eyePos );

        const Vector3f& circuitCenter = volumeBBox.getCenter();
        const Vector3f& circuitSize = volumeBBox.getDimension();
        const float isotropicScale = circuitSize.find_max();

        eyePos = ( eyePos * isotropicScale ) + circuitCenter;

        modelView = maths::computeModelViewMatrix( rotation, eyePos );

        const FloatVector matrix( modelView.begin(), modelView.end() );
        _publisher.publish( zeq::hbp::serializeCamera( matrix ));
    }

    void onRequest( const zeq::Event& event )
    {
        const zeq::uint128_t& eventType =
                zeq::vocabulary::deserializeRequest( event );

        if( eventType == zeq::hbp::EVENT_IMAGEJPEG )
            framedata.getFrameSettings()->setGrabFrame( true );
        else if( eventType == zeq::vocabulary::EVENT_VOCABULARY )
        {
            zeq::EventDescriptors vocabulary;
            vocabulary.push_back( zeq::EventDescriptor( zeq::hbp::IMAGEJPEG, zeq::hbp::EVENT_IMAGEJPEG,
                                                        zeq::hbp::SCHEMA_IMAGEJPEG, zeq::PUBLISHER ) );
            vocabulary.push_back( zeq::EventDescriptor( zeq::hbp::CAMERA, zeq::hbp::EVENT_CAMERA,
                                                        zeq::hbp::SCHEMA_CAMERA, zeq::BIDIRECTIONAL ) );
            vocabulary.push_back( zeq::EventDescriptor( zeq::vocabulary::EXIT, zeq::vocabulary::EVENT_EXIT,
                                                        zeq::vocabulary::SCHEMA_EXIT, zeq::PUBLISHER ) );
            const zeq::Event& vocEvent = zeq::vocabulary::serializeVocabulary( vocabulary );
            _vwsPublisher->publish( vocEvent );
        }
        else if( eventType == zeq::vocabulary::EVENT_EXIT )
            config->stopRunning();
        else if( eventType == zeq::hbp::EVENT_FRAME )
            publishFrame();
    }

    // Generic camera (from REST) in meters
    void onCamera( const zeq::Event& event )
    {
        LBASSERT( event.getType() == zeq::hbp::EVENT_CAMERA );
        const std::vector< float >& matrix = zeq::hbp::deserializeCamera(event);
        Matrix4f modelViewMatrix;
        modelViewMatrix.set( matrix.begin(), matrix.end(), false );
        CameraSettingsPtr cameraSettings = framedata.getCameraSettings();
        cameraSettings->setModelViewMatrix( modelViewMatrix );
    }

    // HBP 'micron' camera from other brain applications
    void onHBPCamera( const zeq::Event& event )
    {
        LBASSERT( event.getType() == zeq::hbp::EVENT_CAMERA );
        const std::vector< float >& matrix = zeq::hbp::deserializeCamera(event);

        const Vector3f circuitCenter = volumeBBox.getCenter();
        const Vector3f circuitSize = volumeBBox.getDimension();
        const float isotropicScale = circuitSize.find_max();

        Matrix4f modelViewMatrix;
        modelViewMatrix.set( matrix.begin(), matrix.end(), false );

        Matrix3f rotation;
        Vector3f eyePos;
        maths::getRotationAndEyePositionFromModelView( modelViewMatrix,
                                                       rotation,
                                                       eyePos );

        eyePos = ( eyePos - circuitCenter ) / isotropicScale;
        modelViewMatrix = maths::computeModelViewMatrix( rotation, eyePos );

        CameraSettingsPtr cameraSettings = framedata.getCameraSettings();
        cameraSettings->setModelViewMatrix( modelViewMatrix );
    }

    void onLookupTable1D( const zeq::Event& event )
    {
        const TransferFunction1Dc transferFunction(
            zeq::hbp::deserializeLookupTable1D( event ));
        RenderSettingsPtr renderSettings = framedata.getRenderSettings();
        renderSettings->setTransferFunction( transferFunction );
    }

    void onFrame( const zeq::Event& event )
    {
        const zeq::hbp::data::Frame& frame =
            zeq::hbp::deserializeFrame( event );

        FrameSettingsPtr frameSettings = framedata.getFrameSettings();
        ApplicationParameters& params = config->getApplicationParameters();

        params.frames.x() = frame.start;
        frameSettings->setFrameNumber( frame.current );
        params.frames.y() = frame.end;
        params.animation = frame.delta;
    }

    void publishExit()
    {
        _vwsPublisher->publish( zeq::Event( zeq::vocabulary::EVENT_EXIT ));
    }

    void publishFrame()
    {
        FrameSettingsPtr frameSettings = framedata.getFrameSettings();
        const ApplicationParameters& params =config->getApplicationParameters();
        _publisher.publish( zeq::hbp::serializeFrame(
                                zeq::hbp::data::Frame(
                                    params.frames.x(),
                                    frameSettings->getFrameNumber(),
                                    params.frames.y(),
                                    params.animation )));
    }

    Subscribers subscribers;
    zeq::Publisher _publisher;
    lunchbox::Clock _heartbeatClock;
    PublisherPtr _vwsPublisher;
#ifdef LIVRE_USE_RESTBRIDGE
    boost::shared_ptr< restbridge::RestBridge > _restBridge;
#endif
#endif
};
}

Config::Config( eq::ServerPtr parent )
        : eq::Config( parent )
        , _impl( new detail::Config( this ))
{
    _impl->framedata.initialize( this );
}

Config::~Config()
{
    delete _impl;
}

FrameData& Config::getFrameData()
{
    return _impl->framedata;
}

const FrameData& Config::getFrameData() const
{
    return _impl->framedata;
}

const ApplicationParameters& Config::getApplicationParameters() const
{
    return static_cast<const Client&>( *getClient()).getApplicationParameters();
}

ApplicationParameters& Config::getApplicationParameters()
{
    return static_cast< Client& >( *getClient()).getApplicationParameters();
}

void Config::mapFrameData( const eq::uint128_t& initId )
{
    _impl->framedata.map( this, initId );
    _impl->framedata.mapObjects( );
}

void Config::unmapFrameData()
{
    _impl->framedata.unmapObjects( );
    _impl->framedata.unmap( this );
}

void Config::resetCamera( )
{
    _impl->framedata.getCameraSettings()->reset();

#ifdef LIVRE_USE_ZEQ
    _impl->publishModelView();
#endif

}

bool Config::init()
{
    _initZeroEQ();
    resetCamera();
    _initEvents();
    FrameSettingsPtr frameSettings = _impl->framedata.getFrameSettings();
    const ApplicationParameters& params = getApplicationParameters();
    frameSettings->setFrameNumber( params.frames.x( ));

    _impl->framedata.registerObjects();

    if( !_registerFrameData( ))
        return false;

    if( !eq::Config::init( _impl->framedata.getID( )))
    {
        _impl->framedata.deregisterObjects();
        _deregisterFrameData();
        return false;
    }

    const eq::Canvases& canvases = getCanvases();
    _impl->currentCanvas = canvases.empty() ? 0 : canvases.front();
    return true;
}

uint32_t Config::startFrame()
{
    FrameSettingsPtr frameSettings = _impl->framedata.getFrameSettings();
    const ApplicationParameters& params = getApplicationParameters();
    const uint32_t start = params.frames.x();
    const uint32_t current = frameSettings->getFrameNumber() > start ?
                                 frameSettings->getFrameNumber() : start;
    const uint32_t end = params.frames.y() > current ?
                             params.frames.y() : current;
    const int32_t delta = params.animation;
    // avoid overflow condition:
    const uint32_t interval = end-start == 0xFFFFFFFFu ?
                                  0xFFFFFFFFu : end - start + 1;
    const uint32_t frame = ((current-start+delta) % interval) + start;
    frameSettings->setFrameNumber( frame );

    const eq::uint128_t& version = _impl->framedata.commit();
    frameSettings->setGrabFrame( false );
#ifdef LIVRE_USE_ZEQ
    if( _impl->_heartbeatClock.getTimef() >= DEFAULT_HEARTBEAT_TIME )
    {
        _impl->_heartbeatClock.reset();
        _impl->_vwsPublisher->publish(
            zeq::Event( zeq::vocabulary::EVENT_HEARTBEAT ));
    }
#endif
    return eq::Config::startFrame( version );
}

bool Config::exit()
{
    bool ret = eq::Config::exit();
    // cppcheck-suppress unreachableCode
    _impl->framedata.deregisterObjects();
    if( !_deregisterFrameData() )
        ret = false;
#ifdef LIVRE_USE_RESTBRIDGE
    _impl->publishExit();
#endif

    return ret;
}

bool Config::switchCanvas_( )
{
    FrameSettingsPtr frameSettings = _impl->framedata.getFrameSettings();

    const eq::Canvases& canvases = getCanvases();
    if( canvases.empty( ))
        return true;

    frameSettings->setCurrentViewId( lunchbox::uint128_t( 0 ) );

    if( !_impl->currentCanvas )
    {
        _impl->currentCanvas = canvases.front();
        return true;
    }

    eq::Canvases::const_iterator i = std::find( canvases.begin(),
                                                canvases.end(),
                                                _impl->currentCanvas );
    LBASSERT( i != canvases.end( ));

    ++i;
    if( i == canvases.end( ))
        _impl->currentCanvas = canvases.front();
    else
        _impl->currentCanvas = *i;

    return true;
}

bool Config::switchView_()
{
    FrameSettingsPtr frameSettings = _impl->framedata.getFrameSettings();

    const eq::Canvases& canvases = getCanvases();
    if( !_impl->currentCanvas && !canvases.empty( ) )
        _impl->currentCanvas= canvases.front();

    if( !_impl->currentCanvas )
        return true;

    const eq::Layout* layout = _impl->currentCanvas->getActiveLayout();
    if( !layout )
        return true;

    const eq::View* current = find< eq::View >( frameSettings->getCurrentViewId( ) );

    const eq::Views& views = layout->getViews();
    LBASSERT( !views.empty( ))

    if( !current )
    {
        frameSettings->setCurrentViewId( views.front()->getID( ));
        return true;
    }

    eq::Views::const_iterator i = std::find( views.begin(),
                                                  views.end(),
                                                  current );
    LBASSERT( i != views.end( ));

    ++i;
    if( i == views.end( ))
        frameSettings->setCurrentViewId( lunchbox::uint128_t( 0 ) );
    else
        frameSettings->setCurrentViewId( (*i)->getID( ));

    return true;
}

bool Config::switchToViewCanvas_( const eq::uint128_t& viewID )
{
    FrameSettingsPtr frameSettings = _impl->framedata.getFrameSettings();

    frameSettings->setCurrentViewId( viewID );
    if( viewID == 0 )
    {
        _impl->currentCanvas = 0;
        return false;
    }

    const eq::View* view = find< eq::View >( viewID );
    const eq::Layout* layout = view->getLayout();
    const eq::Canvases& canvases = getCanvases();
    for( eq::CanvasesCIter i = canvases.begin();
         i != canvases.end(); ++i )
    {
        eq::Canvas* canvas = *i;
        const eq::Layout* canvasLayout = canvas->getActiveLayout();

        if( canvasLayout == layout )
        {
            _impl->currentCanvas = canvas;
            return true;
        }
    }
    return true;
}

void Config::handleEvents()
{
    eq::Config::handleEvents();

#ifdef LIVRE_USE_ZEQ
    // Receiving all queued events from all receivers without blocking.
    for( Subscribers::iterator it = _impl->subscribers.begin();
         it != _impl->subscribers.end(); ++it )
    {
        while(( *it )->receive( 0 ))
            /*nop*/ ;
    }
#endif
}

bool Config::handleEvent( const eq::ConfigEvent* event )
{
#ifdef LIVRE_USE_ZEQ
    const Matrix4f& oldModelViewMatrix =
            _impl->framedata.getCameraSettings()->getModelViewMatrix();
#endif

    EqEventInfo eventInfo( this, event );

    bool hasEvent = false;

    switch( event->data.type )
    {
        case eq::Event::KEY_PRESS:
        {
            if( _impl->eventMapper.handleEvent( EVENT_KEYBOARD, eventInfo ) )
                hasEvent = true;
            break;
        }
        case eq::Event::CHANNEL_POINTER_BUTTON_PRESS:
        case eq::Event::CHANNEL_POINTER_BUTTON_RELEASE:
        case eq::Event::CHANNEL_POINTER_MOTION:
        case eq::Event::CHANNEL_POINTER_WHEEL:
        {
            if( _impl->eventMapper.handleEvent( EVENT_CHANNEL_POINTER, eventInfo ))
               hasEvent = true;
            break;
        }

        default:
            break;
    }

    if( hasEvent )
    {
#ifdef LIVRE_USE_ZEQ
        if( _impl->framedata.getCameraSettings()->getModelViewMatrix() != oldModelViewMatrix )
            _impl->detail::Config::publishModelView();
#endif
        return true;
    }


    return eq::Config::handleEvent( event );
}

bool Config::handleEvent( eq::EventICommand command )
{
    switch( command.getEventType( ))
    {
        case VOLUME_BOUNDING_BOX:
            _impl->volumeBBox = command.read< Boxf >();
            return false;
#ifdef LIVRE_USE_ZEQ
        case GRAB_IMAGE:
        {
            const uint64_t dataSize = command.read< uint64_t >();
            const uint8_t* dataPtr =
                    reinterpret_cast< const uint8_t* >( command.getRemainingBuffer( dataSize ) );

            const zeq::hbp::data::ImageJPEG image( dataSize, dataPtr );
            const zeq::Event& image_event = zeq::hbp::serializeImageJPEG( image );
            _impl->_vwsPublisher->publish( image_event );
            return false;
        }
#endif
    }

    return eq::Config::handleEvent( command );
}

bool Config::_registerFrameData()
{
    return _impl->framedata.registerToConfig_( this );
}

bool Config::_deregisterFrameData()
{
    return _impl->framedata.deregisterFromConfig_( this );
}

void Config::_initZeroEQ()
{
#ifdef LIVRE_USE_ZEQ
#  ifdef LIVRE_USE_RESTBRIDGE
    const std::string schema = _impl->framedata.getRESTParameters()->zeqSchema +
                               PUBLISHER_SCHEMA_SUFFIX;

    _impl->_vwsPublisher.reset( new zeq::Publisher( lunchbox::URI( schema )));

    if( _impl->framedata.getRESTParameters()->useRESTBridge )
    {
        _impl->_restBridge.reset(
            new restbridge::RestBridge(
                _impl->framedata.getRESTParameters()->hostName,
                _impl->framedata.getRESTParameters()->port ));
        _impl->_restBridge->run(
            _impl->framedata.getRESTParameters()->zeqSchema );
    }

#  else
    _impl->_vwsPublisher.reset( new zeq::Publisher(
                                    lunchbox::URI( "vwsresp://" )));
#  endif

    if( !servus::Servus::isAvailable( ))
        return;

    SubscriberPtr subscriber( new zeq::Subscriber( lunchbox::URI( "hbp://" )));
    _impl->subscribers.push_back( subscriber );
    if( getApplicationParameters().syncCamera )
    {
        subscriber->registerHandler( zeq::hbp::EVENT_CAMERA,
                                     boost::bind( &detail::Config::onHBPCamera,
                                                 _impl, _1 ));
    }
    subscriber->registerHandler( zeq::hbp::EVENT_LOOKUPTABLE1D,
                                 boost::bind( &detail::Config::onLookupTable1D,
                                              _impl, _1 ));
    subscriber->registerHandler( zeq::hbp::EVENT_FRAME,
                                 boost::bind( &detail::Config::onFrame,
                                              _impl, _1 ));
#  ifdef LIVRE_USE_RESTBRIDGE
    if( !_impl->framedata.getRESTParameters()->useRESTBridge )
        return;

    const std::string subscriberSchema =
        _impl->framedata.getRESTParameters()->zeqSchema +
        SUBSCRIBER_SCHEMA_SUFFIX;

    SubscriberPtr vwsSubscriber(
                new zeq::Subscriber( lunchbox::URI( subscriberSchema )));
    _impl->subscribers.push_back( vwsSubscriber );
    vwsSubscriber->registerHandler( zeq::hbp::EVENT_CAMERA,
                                    boost::bind( &detail::Config::onCamera,
                                                 _impl, _1 ));
    vwsSubscriber->registerHandler( zeq::vocabulary::EVENT_REQUEST,
                                    boost::bind( &detail::Config::onRequest,
                                                 _impl, _1 ));
#  endif
#endif
}

void Config::_initEvents()
{
    _impl->eventMapper.registerEvent( EVENT_CHANNEL_POINTER );
    _impl->eventMapper.registerEvent( EVENT_KEYBOARD );
}

void Config::switchLayout_( const int32_t increment )
{
    if( !_impl->currentCanvas )
        return;

    _impl->framedata.getFrameSettings()->setCurrentViewId( lunchbox::uint128_t(0));

    size_t index = _impl->currentCanvas->getActiveLayoutIndex() + increment;
    const eq::Layouts& layouts = _impl->currentCanvas->getLayouts();
    LBASSERT( !layouts.empty( ));

    index = ( index % layouts.size( ) );
    _impl->currentCanvas->useLayout( uint32_t( index ) );
}

}
