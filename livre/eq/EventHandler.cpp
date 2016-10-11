/* Copyright (c) 2016, Blue Brain Project / EPFL
 *                     bbp-open-source@googlegroups.com
 *                     Stefan.Eilemann@epfl.ch
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

#include <livre/eq/EventHandler.h>

#include <livre/eq/Config.h>
#include <livre/eq/Event.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/serialization.h>
#include <livre/eq/settings/CameraSettings.h>
#include <livre/eq/settings/FrameSettings.h>
#include <livre/eq/settings/RenderSettings.h>
#include <livre/core/data/Histogram.h>
#include <livre/core/data/VolumeInformation.h>

namespace livre
{
namespace
{
const float ROTATE_AND_ZOOM_SPEED = 0.005f;
const float PAN_SPEED = 0.0005f;
const float ADVANCE_SPEED = 0.05f;

class ViewHistogram
{
public:
    ViewHistogram( const Histogram& histogram_, const float area_,
                   const uint32_t id_ )
        : histogram( histogram_ )
        , area( area_ )
        , id( id_ )
    {}

    ViewHistogram& operator+=( const ViewHistogram& hist )
    {
        if( this == &hist )
            return *this;

        histogram += hist.histogram;
        area += hist.area;
        return *this;
    }

    bool isComplete() const
    {
        const float eps = 0.0001f;
        return std::abs( 1.0f - area ) <= eps;
    }

    Histogram histogram;
    float area;
    uint32_t id;
};

typedef std::deque< ViewHistogram > ViewHistogramQueue;
}

template< class C > class EventHandler< C >::Impl
{
public:
    Impl( Config& c )
        : config( c )
        , volumeBBox( Boxf::makeUnitBox( ))
        , _currentCanvas( nullptr )
    {}

    void init()
    {
        const eq::Canvases& canvases = config.getCanvases();
        _currentCanvas = canvases.empty() ? nullptr : canvases.front();
    }

    bool switchCanvas()
    {
        const eq::Canvases& canvases = config.getCanvases();
        if( canvases.empty( ))
            return true;

        FrameSettings& frameSettings = config.getFrameData().getFrameSettings();
        frameSettings.setCurrentViewId( lunchbox::uint128_t( 0 ) );

        if( !_currentCanvas )
        {
            _currentCanvas = canvases.front();
            return true;
        }

        auto i = std::find( canvases.begin(), canvases.end(), _currentCanvas );
        LBASSERT( i != canvases.end( ));

        ++i;
        if( i == canvases.end( ))
            _currentCanvas = canvases.front();
        else
            _currentCanvas = *i;

        return true;
    }

    bool switchView()
    {
        const eq::Canvases& canvases = config.getCanvases();
        if( !_currentCanvas && !canvases.empty( ))
            _currentCanvas = canvases.front();

        if( !_currentCanvas )
            return true;

        const eq::Layout* layout = _currentCanvas->getActiveLayout();
        if( !layout )
            return true;

        FrameSettings& frameSettings = config.getFrameData().getFrameSettings();
        const eq::View* current =
            config.find< eq::View >( frameSettings.getCurrentViewId( ));

        const eq::Views& views = layout->getViews();
        LBASSERT( !views.empty( ));

        if( !current )
        {
            frameSettings.setCurrentViewId( views.front()->getID( ));
            return true;
        }

        auto i = std::find( views.begin(), views.end(), current );
        LBASSERT( i != views.end( ));

        ++i;
        if( i == views.end( ))
            frameSettings.setCurrentViewId( lunchbox::uint128_t( 0 ) );
        else
            frameSettings.setCurrentViewId( (*i)->getID( ));

        return true;
    }

    bool switchToViewCanvas( const eq::uint128_t& viewID )
    {
        FrameSettings& frameSettings = config.getFrameData().getFrameSettings();
        frameSettings.setCurrentViewId( viewID );

        if( viewID == 0 )
        {
            _currentCanvas = 0;
            return false;
        }

        const eq::View* view = config.find< eq::View >( viewID );
        const eq::Layout* layout = view->getLayout();
        const eq::Canvases& canvases = config.getCanvases();
        for( eq::Canvas* canvas : canvases )
        {
            const eq::Layout* canvasLayout = canvas->getActiveLayout();
            if( canvasLayout == layout )
            {
                _currentCanvas = canvas;
                return true;
            }
        }
        return true;
    }

    void switchLayout( const int32_t increment )
    {
        if( !_currentCanvas )
            return;

        config.getFrameData().getFrameSettings().setCurrentViewId(
            lunchbox::uint128_t( 0 ));

        size_t index = _currentCanvas->getActiveLayoutIndex() + increment;
        const eq::Layouts& layouts = _currentCanvas->getLayouts();
        LBASSERT( !layouts.empty( ));

        index = ( index % layouts.size( ));
        _currentCanvas->useLayout( uint32_t( index ));
    }

    void gatherHistogram( const Histogram& histogram, const float area,
                          const uint32_t currentId )
    {
        // If we get a very old frame skip it
        if( !histogramQueue.empty() && currentId < histogramQueue.back().id )
            return;

        const ViewHistogram viewHistogram( histogram, area, currentId );
        for( auto it = histogramQueue.begin(); it != histogramQueue.end(); )
        {
            auto& data = *it;
            bool dataMerged = false;

            if( currentId == data.id )
            {
                dataMerged = true;
                data += viewHistogram;
            }
            else if( currentId > data.id )
            {
                dataMerged = true;
                it = histogramQueue.emplace( it, viewHistogram );
            }

            if( data.isComplete( )) // Send histogram & remove all old ones
            {
#ifdef LIVRE_USE_ZEROEQ
                config.publish( data.histogram );
#endif
                histogramQueue.erase( it, histogramQueue.end( ));
                return;
            }

            if( dataMerged )
                break;
            ++it;
        }

        if( histogramQueue.empty() && !viewHistogram.isComplete( ))
        {
            histogramQueue.push_back( viewHistogram );
            return;
        }

        if( viewHistogram.isComplete( ))
        {
#ifdef LIVRE_USE_ZEROEQ
            config.publish( viewHistogram.histogram );
#endif
            return;
        }

        if( histogramQueue.size() > config.getLatency() + 1 )
            histogramQueue.pop_back();
    }


    Config& config;
    Boxf volumeBBox;
    ViewHistogramQueue histogramQueue;

private:
    eq::Canvas* _currentCanvas;
};

template< class C > template< class... ARGS >
EventHandler< C >::EventHandler( Config& config, ARGS... args )
    : C( args... )
    , _impl( new Impl( config ))
{}

template< class C > EventHandler< C >::~EventHandler()
{}

template< class C > void EventHandler< C >::init()
{
    _impl->init();
}

template< class C >
bool EventHandler< C >::handleEvent( const eq::EventType type,
                                     const eq::KeyEvent& event )
{
    if( type != eq::EVENT_KEY_PRESS )
        return eq::Config::handleEvent( type, event );

    FrameSettings& frameSettings =
        _impl->config.getFrameData().getFrameSettings();

    switch( event.key )
    {
    case ' ':
        _impl->config.resetCamera();
        return true;

    case 's':
    case 'S':
        frameSettings.toggleStatistics();
        return true;

    case 'a':
    case 'A':
        _impl->config.getFrameData().getRenderSettings().toggleDrawAxis();
        return true;

    case 'i':
    case 'I':
        frameSettings.toggleInfo();
        return true;

    case 'l':
        _impl->switchLayout( 1 );
        return true;

    case 'L':
        _impl->switchLayout( -1 );
        return true;

    case 'c':
    case 'C':
        return _impl->switchCanvas();

    case 'v':
    case 'V':
        return _impl->switchView();

    default:
        return eq::Config::handleEvent( type, event );
    }
}

template< class C >
bool EventHandler< C >::handleEvent( const eq::EventType type,
                                     const eq::PointerEvent& event )
{
    CameraSettings& camera = _impl->config.getFrameData().getCameraSettings();

    switch( type )
    {
    case eq::EVENT_CHANNEL_POINTER_BUTTON_PRESS:
        return _impl->switchToViewCanvas( event.context.view.identifier );

    case eq::EVENT_CHANNEL_POINTER_MOTION:
        switch( event.buttons )
        {
        case eq::PTR_BUTTON1:
            camera.spinModel( -ROTATE_AND_ZOOM_SPEED * event.dy,
                              -ROTATE_AND_ZOOM_SPEED * event.dx );
            return true;

        case eq::PTR_BUTTON2:
            camera.moveCamera( 0.f, 0.f, ROTATE_AND_ZOOM_SPEED * -event.dy );
            return true;

        case eq::PTR_BUTTON3:
            camera.moveCamera( PAN_SPEED * event.dx, -PAN_SPEED * event.dy,
                               0.f );
            return true;
        }
        return eq::Config::handleEvent( type, event );

    case eq::EVENT_CHANNEL_POINTER_WHEEL:
        camera.moveCamera( -ADVANCE_SPEED * event.xAxis, 0.f,
                            ADVANCE_SPEED * event.yAxis );
        return true;

    default:
        return eq::Config::handleEvent( type, event );
    }
}

template< class C >
bool EventHandler< C >::handleEvent( eq::EventICommand command )
{
    switch( command.getEventType( ))
    {
    case HISTOGRAM_DATA:
    {
        const Histogram& histogram = command.read< Histogram >();
        const float area = command.read< float >();
        const uint32_t id = command.read< uint32_t >();
        _impl->gatherHistogram( histogram, area, id );
        return false;
    }

    case VOLUME_INFO:
        command >> _impl->config.getVolumeInformation();
        return false;

    case REDRAW:
        _impl->config.postRedraw();
        return true;
    }

    if( !eq::Config::handleEvent( command ))
        return false;

    _impl->config.postRedraw();
    return true;
}

}

// Instantiate:
template class livre::EventHandler< eq::Config >;
template livre::EventHandler< eq::Config >::EventHandler( livre::Config&,
                                                          eq::ServerPtr );
