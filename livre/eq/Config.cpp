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

#include <livre/eq/Config.h>

#include <livre/eq/Client.h>
#include <livre/eq/Event.h>
#include <livre/eq/events/EqEventHandlerFactory.h>
#include <livre/eq/events/EqEventInfo.h>
#include <livre/eq/events/Events.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/settings/CameraSettings.h>
#include <livre/eq/settings/FrameSettings.h>
#include <livre/eq/settings/RenderSettings.h>
#include <livre/core/events/EventMapper.h>
#include <livre/core/maths/maths.h>

#ifdef LIVRE_USE_ZEQ
#  include <livre/eq/zeq/communicator.h>
#endif

#include <eq/eq.h>

namespace livre
{
class Config::Impl
{
public:
    Impl( Config* config_ )
        : config( config_ )
        , currentCanvas( 0 )
        , eventMapper( EventHandlerFactoryPtr( new EqEventHandlerFactory ))
        , volumeBBox( Boxf::makeUnitBox( ))
    {}

    void publishModelView()
    {
#ifdef LIVRE_USE_ZEQ
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

        communicator->publishModelView( modelView );
#endif
    }

    Matrix4f convertFromHBPCamera( const Matrix4f& modelViewMatrix ) const
    {
        const Vector3f circuitCenter = volumeBBox.getCenter();
        const Vector3f circuitSize = volumeBBox.getDimension();
        const float isotropicScale = circuitSize.find_max();

        Matrix3f rotation;
        Vector3f eyePos;
        maths::getRotationAndEyePositionFromModelView( modelViewMatrix,
                                                       rotation,
                                                       eyePos );

        eyePos = ( eyePos - circuitCenter ) / isotropicScale;
        return maths::computeModelViewMatrix( rotation, eyePos );
    }

    Config* config;
    eq::Canvas* currentCanvas;
    EventMapper eventMapper;
    FrameData framedata;
    Boxf volumeBBox;
#ifdef LIVRE_USE_ZEQ
    std::unique_ptr< zeq::Communicator > communicator;
#endif
};

Config::Config( eq::ServerPtr parent )
    : eq::Config( parent )
    , _impl( new Impl( this ))
{
    _impl->framedata.initialize( this );
}

Config::~Config()
{
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

void Config::resetCamera()
{
    _impl->framedata.getCameraSettings()->reset();

    _impl->publishModelView();
}

bool Config::init()
{
#ifdef LIVRE_USE_ZEQ
    _impl->communicator.reset( new zeq::Communicator( *this ));
#endif

    resetCamera();
    _initEvents();
    FrameSettingsPtr frameSettings = _impl->framedata.getFrameSettings();
    const ApplicationParameters& params = getApplicationParameters();
    frameSettings->setFrameNumber( params.frames.x( ));

    RenderSettingsPtr renderSettings = _impl->framedata.getRenderSettings();
    const TransferFunction1Dc& tf( _impl->framedata.getVRParameters()->transferFunction );
    renderSettings->setTransferFunction( tf );

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

uint32_t Config::frame()
{
    // Set current frame (start/end may have changed)
    FrameSettingsPtr frameSettings = _impl->framedata.getFrameSettings();
    const ApplicationParameters& params = getApplicationParameters();
    const uint32_t start = params.frames.x();
    const uint32_t current = frameSettings->getFrameNumber() > start ?
                                 frameSettings->getFrameNumber() : start;

    frameSettings->setFrameNumber( current );
    const eq::uint128_t& version = _impl->framedata.commit();

    // reset data and advance current frame
    frameSettings->setGrabFrame( false );
    const uint32_t end = params.frames.y() > current ?
                             params.frames.y() : current;
    const int32_t delta = params.animation;
    // avoid overflow condition:
    const uint32_t interval = end-start == 0xFFFFFFFFu ?
                                  0xFFFFFFFFu : end - start + 1;
    const uint32_t frameNumber = ((current-start+delta) % interval) + start;
    frameSettings->setFrameNumber( frameNumber );

#ifdef LIVRE_USE_ZEQ
    _impl->communicator->publishHeartbeat();
#endif
    eq::Config::startFrame( version );
    return eq::Config::finishFrame();
}

bool Config::exit()
{
    bool ret = eq::Config::exit();
    // cppcheck-suppress unreachableCode
    _impl->framedata.deregisterObjects();
    if( !_deregisterFrameData() )
        ret = false;
#ifdef LIVRE_USE_ZEQ
    _impl->communicator->publishExit();
#endif

    return ret;
}

bool Config::switchCanvas( )
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

bool Config::switchView()
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

bool Config::switchToViewCanvas( const eq::uint128_t& viewID )
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
    _impl->communicator->handleEvents();
#endif
}

Matrix4f Config::convertFromHBPCamera( const Matrix4f& modelViewMatrix ) const
{
    return _impl->convertFromHBPCamera( modelViewMatrix );
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
            _impl->publishModelView();
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

            _impl->communicator->publishImageJPEG( dataPtr, dataSize );
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

void Config::_initEvents()
{
    _impl->eventMapper.registerEvent( EVENT_CHANNEL_POINTER );
    _impl->eventMapper.registerEvent( EVENT_KEYBOARD );
}

void Config::switchLayout( const int32_t increment )
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
