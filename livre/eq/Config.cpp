/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <livre/lib/configuration/ApplicationParameters.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>

#include <livre/core/events/EventMapper.h>
#include <livre/core/maths/maths.h>
#include <livre/core/util/FrameUtils.h>

#include <zerobuf/render/imageJPEG.h>

#ifdef LIVRE_USE_ZEROEQ
#  include <livre/eq/zeroeq/communicator.h>
#endif

#include <eq/eq.h>

namespace livre
{
class Config::Impl
{
public:
    explicit Impl( Config* config_ )
        : config( config_ )
        , defaultLatency( 0 )
        , currentCanvas( 0 )
        , eventMapper( EventHandlerFactoryPtr( new EqEventHandlerFactory ))
        , volumeBBox( Boxf::makeUnitBox( ))
        , redraw( true )
        , dataFrameRange( INVALID_FRAME_RANGE )
    {}

    Config* config;
    uint32_t defaultLatency;
    eq::Canvas* currentCanvas;
    EventMapper eventMapper;
    FrameData framedata;
    Boxf volumeBBox;
    ::zerobuf::render::ImageJPEG imageJPEG;
#ifdef LIVRE_USE_ZEROEQ
    std::unique_ptr< zeroeq::Communicator > communicator;
#endif
    bool redraw;
    Vector2ui dataFrameRange;
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

::zerobuf::render::ImageJPEG& Config::getImageJPEG() const
{
    return _impl->imageJPEG;
}

void Config::renderJPEG()
{
    _impl->imageJPEG.setData( std::vector< uint8_t >( ));
    getFrameData().getFrameSettings().setGrabFrame( true );
    frame();

    while( _impl->imageJPEG.getData().empty())
    {
        eq::EventICommand event = getNextEvent();

        if( !event.isValid( ))
            continue;

        handleEvent( event );
    }
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
    _impl->framedata.getCameraSettings().setCameraPosition(
        getApplicationParameters().cameraPosition );
    _impl->framedata.getCameraSettings().setCameraLookAt(
        getApplicationParameters().cameraLookAt );
#ifdef LIVRE_USE_ZEROEQ
    _impl->communicator->publishCamera();
#endif
}

bool Config::init( const int argc LB_UNUSED, char** argv LB_UNUSED )
{
#ifdef LIVRE_USE_ZEROEQ
    _impl->communicator.reset( new zeroeq::Communicator( *this, argc, argv ));
#endif

    resetCamera();
    _initEvents();
    FrameData& framedata = _impl->framedata;
    FrameSettings& frameSettings = framedata.getFrameSettings();
    const ApplicationParameters& params = getApplicationParameters();
    frameSettings.setFrameNumber( params.frames.x( ));

    RenderSettings& renderSettings = framedata.getRenderSettings();
    const TransferFunction1D tf( params.transferFunction );
    renderSettings.setTransferFunction( tf );

    _impl->framedata.registerObjects();

    if( !_registerFrameData( ))
        return false;

    const VolumeRendererParameters& vrParameters = framedata.getVRParameters();
    if( vrParameters.getSynchronousMode( ))
        setLatency( 0 );

    if( !eq::Config::init( _impl->framedata.getID( )))
    {
        _impl->framedata.deregisterObjects();
        _deregisterFrameData();
        return false;
    }

    _impl->defaultLatency = getLatency();

    const eq::Canvases& canvases = getCanvases();
    _impl->currentCanvas = canvases.empty() ? 0 : canvases.front();
    return true;
}

bool Config::frame()
{
    if( _impl->dataFrameRange == INVALID_FRAME_RANGE )
        return false;

    ApplicationParameters& params = getApplicationParameters();
    FrameSettings& frameSettings = _impl->framedata.getFrameSettings();

    const FrameUtils frameUtils( params.frames, _impl->dataFrameRange );
    params.frames = frameUtils.getFrameRange();

    // Set current frame (start/end may have changed)
    const bool keepToLatest = params.animation == LATEST_FRAME;
    const uint32_t current =
            frameUtils.getCurrent( frameSettings.getFrameNumber(), keepToLatest );

    frameSettings.setFrameNumber( current );
    const eq::uint128_t& version = _impl->framedata.commit();

    if( _impl->framedata.getVRParameters().getSynchronousMode( ))
        setLatency( 0 );
    else
        setLatency( _impl->defaultLatency );

    // reset data and advance current frame
    frameSettings.setGrabFrame( false );

    if( !keepToLatest )
        frameSettings.setFrameNumber( frameUtils.getNext( current,
                                                          params.animation ));
    _impl->redraw = false;

#ifdef LIVRE_USE_ZEROEQ
    _impl->communicator->publishFrame();
#endif

    eq::Config::startFrame( version );
    eq::Config::finishFrame();
    return true;
}

uint32_t Config::getDataFrameCount() const
{
    const Vector2ui& range = _impl->dataFrameRange;
    return range[1] > range[0] ? range[1] - range[0] : 0;
}

bool Config::needRedraw()
{
    return _impl->redraw || getApplicationParameters().animation != 0;
}

bool Config::exit()
{
    bool ret = eq::Config::exit(); // cppcheck-suppress unreachableCode
    _impl->framedata.deregisterObjects();
    if( !_deregisterFrameData() )
        ret = false;
#ifdef LIVRE_USE_ZEROEQ
    _impl->communicator->publishExit();
#endif

    return ret;
}

bool Config::switchCanvas()
{
    const eq::Canvases& canvases = getCanvases();
    if( canvases.empty( ))
        return true;

    FrameSettings& frameSettings = _impl->framedata.getFrameSettings();
    frameSettings.setCurrentViewId( lunchbox::uint128_t( 0 ) );

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
    const eq::Canvases& canvases = getCanvases();
    if( !_impl->currentCanvas && !canvases.empty( ) )
        _impl->currentCanvas = canvases.front();

    if( !_impl->currentCanvas )
        return true;

    const eq::Layout* layout = _impl->currentCanvas->getActiveLayout();
    if( !layout )
        return true;

    FrameSettings& frameSettings = _impl->framedata.getFrameSettings();
    const eq::View* current = find< eq::View >( frameSettings.getCurrentViewId( ));

    const eq::Views& views = layout->getViews();
    LBASSERT( !views.empty( ))

    if( !current )
    {
        frameSettings.setCurrentViewId( views.front()->getID( ));
        return true;
    }

    eq::Views::const_iterator i = std::find( views.begin(), views.end(),
                                             current );
    LBASSERT( i != views.end( ));

    ++i;
    if( i == views.end( ))
        frameSettings.setCurrentViewId( lunchbox::uint128_t( 0 ) );
    else
        frameSettings.setCurrentViewId( (*i)->getID( ));

    return true;
}

bool Config::switchToViewCanvas( const eq::uint128_t& viewID )
{
    FrameSettings& frameSettings = _impl->framedata.getFrameSettings();
    frameSettings.setCurrentViewId( viewID );

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

void Config::handleNetworkEvents()
{
#ifdef LIVRE_USE_ZEROEQ
    _impl->communicator->handleEvents();
    _impl->communicator->publishHeartbeat();
#endif
}

bool Config::handleEvent( const eq::ConfigEvent* event )
{
#ifdef LIVRE_USE_ZEROEQ
    CameraSettings cameraSettings = _impl->framedata.getCameraSettings();
#endif

    EqEventInfo eventInfo( this, event );
    bool hasEvent = false;

    switch( event->data.type )
    {
    case eq::Event::KEY_PRESS:
    {
        if( _impl->eventMapper.handleEvent( EVENT_KEYBOARD, eventInfo ))
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
#ifdef LIVRE_USE_ZEROEQ
        if( _impl->framedata.getCameraSettings() != cameraSettings )
            _impl->communicator->publishCamera();
#endif
        _impl->redraw = true;
        return true;
    }

    _impl->redraw |= eq::Config::handleEvent( event );
    return _impl->redraw;
}

bool Config::handleEvent( eq::EventICommand command )
{
    switch( command.getEventType( ))
    {
    case VOLUME_BOUNDING_BOX:
        _impl->volumeBBox = command.read< Boxf >();
        return false;

#ifdef LIVRE_USE_ZEROEQ
    case GRAB_IMAGE:
    {
        const uint64_t dataSize = command.read< uint64_t >();
        const uint8_t* dataPtr =
            reinterpret_cast< const uint8_t* >( command.getRemainingBuffer( dataSize ) );

        _impl->imageJPEG.setData( dataPtr, dataSize );
        return false;
    }
#endif
    case VOLUME_FRAME_RANGE:
    {
        _impl->dataFrameRange = command.read< Vector2ui >();
        return false;
    }

    case REDRAW:
        _impl->redraw = true;
        return true;
    }

    _impl->redraw |= eq::Config::handleEvent( command );
    return _impl->redraw;
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

    _impl->framedata.getFrameSettings().setCurrentViewId( lunchbox::uint128_t(0));

    size_t index = _impl->currentCanvas->getActiveLayoutIndex() + increment;
    const eq::Layouts& layouts = _impl->currentCanvas->getLayouts();
    LBASSERT( !layouts.empty( ));

    index = ( index % layouts.size( ) );
    _impl->currentCanvas->useLayout( uint32_t( index ) );
}

}
