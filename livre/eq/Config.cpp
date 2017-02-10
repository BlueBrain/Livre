/* Copyright (c) 2006-2017, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <livre/eq/EventHandler.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/settings/CameraSettings.h>
#include <livre/eq/settings/FrameSettings.h>
#include <livre/eq/settings/RenderSettings.h>
#include <livre/eq/settings/VolumeSettings.h>
#include <livre/lib/configuration/ApplicationParameters.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>

#include <livre/core/data/VolumeInformation.h>
#include <livre/core/util/FrameUtils.h>

#include <lexis/render/imageJPEG.h>

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
        , frameStart( config->getTime( ))
    {}

    void switchLayout( const int32_t increment )
    {
        const eq::Canvases& canvases = config->getCanvases();
        if( canvases.empty( ))
            return;

        auto currentCanvas = canvases.front();
        size_t index = currentCanvas->getActiveLayoutIndex() + increment;
        const eq::Layouts& layouts = currentCanvas->getLayouts();
        LBASSERT( !layouts.empty( ));

        index = ( index % layouts.size( ));
        currentCanvas->useLayout( uint32_t( index ));
        activeLayout = currentCanvas->getActiveLayout();
    }

    Config* config;
    uint32_t latency = 0;
    FrameData framedata;
#ifdef LIVRE_USE_ZEROEQ
    std::unique_ptr< zeroeq::Communicator > communicator;
#endif
    bool redraw = true;
    VolumeInformation volumeInfo;
    int64_t frameStart;

    eq::Layout* activeLayout = nullptr;
    Histogram _histogram;
};

Config::Config( eq::ServerPtr parent )
    : EventHandler< eq::Config >( *this, parent )
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

std::string Config::renderJPEG()
{
    getFrameData().getFrameSettings().setGrabFrame( true );
    frame();

    for( ;; )
    {
        eq::EventICommand event = getNextEvent();
        if( !event.isValid( ))
            continue;

        if( event.getEventType() == GRAB_IMAGE )
        {
            const uint64_t size = event.read< uint64_t >();
            const uint8_t* data = reinterpret_cast< const uint8_t* >(
                                      event.getRemainingBuffer( size ));

            ::lexis::render::ImageJPEG imageJPEG;
            imageJPEG.setData( data, size );
            return imageJPEG.toJSON();
        }

        handleEvent( event );
    }
    return "";
}

void Config::setHistogram( const Histogram& histogram )
{
    _impl->_histogram = histogram;
}

const Histogram& Config::getHistogram() const
{
    return _impl->_histogram;
}

const VolumeInformation& Config::getVolumeInformation() const
{
    return _impl->volumeInfo;
}

VolumeInformation& Config::getVolumeInformation()
{
    return _impl->volumeInfo;
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
        _impl->framedata.getApplicationParameters().cameraPosition );
    _impl->framedata.getCameraSettings().setCameraLookAt(
        _impl->framedata.getApplicationParameters().cameraLookAt );
}

bool Config::init()
{
    resetCamera();
    FrameData& framedata = _impl->framedata;
    FrameSettings& frameSettings = framedata.getFrameSettings();
    const ApplicationParameters& params = framedata.getApplicationParameters();
    frameSettings.setFrameNumber( params.frames.x( ));

    RenderSettings& renderSettings = framedata.getRenderSettings();
    const TransferFunction1D tf( params.transferFunction );
    renderSettings.setTransferFunction( tf );

    _impl->framedata.registerObjects();

    if( !_impl->framedata.registerToConfig( this ))
        return false;

    if( !eq::Config::init( _impl->framedata.getID( )))
    {
        _impl->framedata.deregisterObjects();
        _impl->framedata.deregisterFromConfig( this );
        return false;
    }

    _impl->switchLayout( 0 ); // update active layout
    _impl->latency = getLatency();
    return true;
}

void Config::initCommunicator( const int argc LB_UNUSED, char** argv LB_UNUSED )
{
#ifdef LIVRE_USE_ZEROEQ
    _impl->communicator.reset( new zeroeq::Communicator( *this, argc, argv ));

    _impl->framedata.getCameraSettings().registerNotifyChanged(
                std::bind( &zeroeq::Communicator::publishCamera,
                           _impl->communicator.get(), std::placeholders::_1 ));
#endif
}

bool Config::frame()
{
    if( _impl->volumeInfo.frameRange == INVALID_FRAME_RANGE )
        return false;

    ApplicationParameters& params = _impl->framedata.getApplicationParameters();
    FrameSettings& frameSettings = _impl->framedata.getFrameSettings();

    const FrameUtils frameUtils( params.frames, _impl->volumeInfo.frameRange );
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
        setLatency( _impl->latency );

    // reset data and advance current frame
    frameSettings.setGrabFrame( false );

    if( !keepToLatest && !_keepCurrentFrame( params.animationFPS ))
    {
        frameSettings.setFrameNumber( frameUtils.getNext( current,
                                                          params.animation ));
        // reset starting time for new frame
         _impl->frameStart = getTime();
    }
    _impl->redraw = false;

#ifdef LIVRE_USE_ZEROEQ
    if( _impl->communicator )
        _impl->communicator->publishFrame();
#endif

    eq::Config::startFrame( version );
    eq::Config::finishFrame();
    return true;
}

void Config::postRedraw()
{
    _impl->redraw = true;
}

bool Config::needRedraw()
{
    return _impl->redraw || _impl->framedata.getApplicationParameters().animation != 0;
}

bool Config::publish( const servus::Serializable& serializable )
{
#ifdef LIVRE_USE_ZEROEQ
    if( _impl->communicator )
        return _impl->communicator->publish( serializable );
#endif
    return false;
}


void Config::switchLayout( const int32_t increment )
{
    return _impl->switchLayout( increment );
}

eq::Layout* Config::getActiveLayout()
{
    return _impl->activeLayout;
}

bool Config::exit()
{
    bool ret = eq::Config::exit(); // cppcheck-suppress unreachableCode
    _impl->framedata.deregisterObjects();
    if( !_impl->framedata.deregisterFromConfig( this ))
        ret = false;
    return ret;
}

void Config::handleNetworkEvents()
{
#ifdef LIVRE_USE_ZEROEQ
    if( _impl->communicator )
        _impl->communicator->handleEvents();
#endif
}

bool Config::_keepCurrentFrame( const uint32_t fps ) const
{
    if( fps == 0 )
        return false;

    const double desiredTime = 1.0 / fps;
    const int64_t end = getTime();

    // If the frame duration is shorter than the desired frame time then the
    // current frame should be kept until the duration matches (or exceeds) the
    // expected. Otherwise, the frame number should be normally increased.
    // This means that no frames are artificially skipped due to the fps limit
    const double frameDuration = ( end - _impl->frameStart ) / 1e3;
    return frameDuration < desiredTime;
}

}
