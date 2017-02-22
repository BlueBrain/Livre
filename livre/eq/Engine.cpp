/* Copyright (c) 2017, EPFL/Blue Brain Project
 *                     bbp-open-source@googlegroups.com
 *                     Daniel.Nachbaur@epfl.ch
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

#include "Engine.h"

#include <livre/core/version.h>
#include <livre/core/data/DataSource.h>

#include <livre/lib/configuration/ApplicationParameters.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>


#include <eq/eq.h>
#include <livre/eq/Channel.h>
#include <livre/eq/Client.h>
#include <livre/eq/Config.h>
#include <livre/eq/Error.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/Node.h>
#include <livre/eq/Pipe.h>
#include <livre/eq/Window.h>

namespace
{
class NodeFactory : public eq::NodeFactory
{
public:
    NodeFactory()
    {}

    virtual eq::Config* createConfig( eq::ServerPtr parent )
        { return new livre::Config( parent ); }

    virtual eq::Node* createNode( eq::Config* parent )
        { return new livre::Node( parent ); }

    virtual eq::Pipe* createPipe( eq::Node* parent )
        { return new livre::Pipe( parent ); }

    virtual eq::Window* createWindow( eq::Pipe* parent )
        { return new livre::Window( parent ); }

    virtual eq::Channel* createChannel( eq::Window* parent )
        { return new livre::Channel( parent ); }
};
}

namespace livre
{
struct Engine::Impl
{
    Impl( int argc, char** argv )
    {
        _initParams( argc, const_cast< const char** >( argv ));

        livre::initErrors( );

        if( !eq::init( argc, argv, &nodeFactory ))
        {
            eq::exit();
            livre::exitErrors();
            LBTHROW( std::runtime_error( "Equalizer init failed" ));
        }

        DataSource::loadPlugins();

        client = new livre::Client( argc, argv, _applicationParameters.isResident );
        config = client->chooseConfig();
        if( !config )
            LBTHROW( std::runtime_error( "No matching config on server" ));

        // framedata setup gets volume URI from application params which is
        // needed in Node::configInit() to load volume
        FrameData& frameData = config->getFrameData();
        frameData.setup( _applicationParameters, _rendererParameters );

        if( !config->init( ))
        {
            client->releaseConfig( config );
            config = nullptr;
            LBTHROW( std::runtime_error( "Config init failed" ));
        }
    }

    ~Impl()
    {
        if( config )
        {
            config->exit();
            client->releaseConfig( config );
        }

        eq::exit();
        livre::exitErrors();
    }

    void _initParams( const int argc, const char** argv )
    {
        if( !_applicationParameters.initialize( argc, argv ) ||
            !_rendererParameters.initialize( argc, argv ))
        {
            LBTHROW( std::runtime_error( "Error parsing command line arguments" ));
        }

        if( _applicationParameters.dataFileName.empty())
            _applicationParameters.dataFileName = "mem:///#4096,4096,4096,40";
    }

    NodeFactory nodeFactory;
    lunchbox::RefPtr< livre::Client > client;
    Config* config = nullptr;
    ApplicationParameters _applicationParameters;
    VolumeRendererParameters _rendererParameters;
};

Engine::Engine( int argc, char** argv )
    : _impl( new Engine::Impl( argc, argv ))
{}

Engine::~Engine()
{}

std::string Engine::getHelp()
{
    DataSource::loadPlugins(); // needed to complete --volume

    VolumeRendererParameters vrParameters;
    ApplicationParameters applicationParameters;

    Configuration conf;
    conf.addDescription( vrParameters.getConfiguration( ));
    conf.addDescription( applicationParameters.getConfiguration( ));

    std::stringstream os;
    os << conf;
    return os.str();
}

std::string Engine::getVersion()
{
    std::stringstream os;
    os << "Livre version " << livrecore::Version::getString() << std::endl;
    return os.str();
}

void Engine::run( const int argc, char** argv )
{
    _impl->config->initCommunicator( argc, argv );

    uint32_t maxFrames = _impl->_applicationParameters.maxFrames;
    while( _impl->config->isRunning() && maxFrames-- )
    {
        if( !_impl->config->frame()) // If not valid, reset maxFrames
            maxFrames++;

        if( _impl->client->getIdleFunction( ))
            _impl->client->getIdleFunction()(); // order is important to latency

        // wait for an event requiring redraw
        while( !_impl->config->needRedraw( ))
        {
            // execute non-critical pending commands
            if( _impl->client->processCommands( ))
                _impl->config->handleEvents(); // non-blocking
            else  // no pending commands, block on user event
            {
                // Poll ZeroEq subscribers at least every 100 ms in handleEvents
                const eq::EventICommand& event = _impl->config->getNextEvent( 100 );
                if( event.isValid( ))
                    _impl->config->handleEvent( event );
                _impl->config->handleEvents(); // non-blocking
                _impl->config->handleNetworkEvents(); //blocking
            }
        }
        _impl->config->handleEvents(); // process all pending events
        _impl->config->handleNetworkEvents(); //blocking
    }
}

bool Engine::render( const eq::View::ScreenshotFunc& func )
{
    auto layout = _impl->config->getActiveLayout();
    auto view = layout ? layout->getViews()[0] : nullptr;
    if( view )
        view->enableScreenshot( eq::Frame::Buffer::color, func );

    _impl->config->frame();
    if( _impl->client->getIdleFunction( ))
        _impl->client->getIdleFunction()(); // order is important to latency
    _impl->config->handleEvents();

    if( view )
        view->disableScreenshot();

    return _impl->config->needRedraw();
}

void Engine::resize( const Vector2ui& size )
{
    _impl->config->finishAllFrames();
    auto layout = _impl->config->getActiveLayout();
    if( layout )
        layout->setPixelViewport( eq::PixelViewport{ 0, 0, int32_t(size.x()),
                                                     int32_t(size.y( )) });;
}

const Histogram& Engine::getHistogram() const
{
    return _impl->config->getHistogram();
}

FrameData& Engine::getFrameData()
{
    return _impl->config->getFrameData();
}

const VolumeInformation& Engine::getVolumeInformation() const
{
    return _impl->config->getVolumeInformation();
}

ApplicationParameters& Engine::getApplicationParameters()
{
    return _impl->_applicationParameters;
}

}
