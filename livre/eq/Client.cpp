
/* Copyright (c) 2006-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Maxim Makhinya  <maxmah@gmail.com>
 *                    2013, Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
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

#include <eq/eq.h>

#include <livre/core/version.h>
#include <livre/core/data/VolumeDataSource.h>

#include <livre/lib/configuration/ApplicationParameters.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>

#include <livre/eq/Client.h>
#include <livre/eq/Config.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/settings/VolumeSettings.h>
#include <livre/eq/settings/CameraSettings.h>
#ifdef LIVRE_USE_RESTBRIDGE
#  include <restbridge/RestBridge.h>
#endif

namespace livre
{

struct Client::Impl
{
    Impl()
    {
    }

    ~Impl()
    {
        VolumeDataSource::unloadPlugins();
    }

    bool parseArguments( const int32_t argc, char** argv )
    {
        if( !_applicationParameters.initialize( argc, argv ) ||
            !_rendererParameters.initialize( argc, argv ))
        {
            LBERROR << "Error parsing command line arguments" << std::endl;
            return false;
        }

        if( _applicationParameters.dataFileName.empty())
            _applicationParameters.dataFileName = "mem:///#4096,4096,4096,32";

        return true;
    }

    static std::string getHelp()
    {
        VolumeRendererParameters vrParameters;
        ApplicationParameters applicationParameters;

        Configuration conf;
        conf.addDescription( vrParameters.getConfiguration( ));
        conf.addDescription( applicationParameters.getConfiguration( ));

        std::stringstream os;
        os << conf;
    #ifdef LIVRE_USE_RESTBRIDGE
        os << std::endl << restbridge::RestBridge::getHelp();
    #endif
        return os.str();
    }

    static std::string getVersion()
    {
        std::stringstream os;
        os << "Livre version " << livrecore::Version::getString() << std::endl;
        return os.str();
    }

    bool initLocal( const int argc, char** argv )
    {
        if( !parseArguments( argc, argv ))
            return false;

        VolumeDataSource::loadPlugins(); //initLocal on render clients never returns
        return true;
    }

    IdleFunc _idleFunc;
    ApplicationParameters _applicationParameters;
    VolumeRendererParameters _rendererParameters;
};

Client::Client()
    : _impl( new Client::Impl( ))
{}

Client::~Client()
{}

std::string Client::getHelp()
{
    return Impl::getHelp();
}

std::string Client::getVersion()
{
    return Impl::getVersion();
}

void Client::setIdleFunction( const IdleFunc& idleFunc )
{
    _impl->_idleFunc = idleFunc;
}

const ApplicationParameters& Client::getApplicationParameters() const
{
    return _impl->_applicationParameters;
}

ApplicationParameters& Client::getApplicationParameters()
{
    return _impl->_applicationParameters;
}

bool Client::initLocal( const int argc, char** argv )
{
    if( !_impl->initLocal( argc, argv ))
        return false;

    addActiveLayout( "Simple" ); // prefer single GPU layout by default
    return eq::Client::initLocal( argc, argv );
}

int Client::run( const int argc, char** argv )
{
    // 0. Init local client
    if( !initLocal( argc, argv ))
    {
        LBERROR << "Can't init client" << std::endl;
        return EXIT_FAILURE;
    }

    // 1. connect to server
    eq::ServerPtr server = new eq::Server;
    if( !connectServer( server ))
    {
        LBERROR << "Can't open server" << std::endl;
        exitLocal();
        return EXIT_FAILURE;
    }

    // 2. choose config
    eq::fabric::ConfigParams configParams;

    Config* config = static_cast< Config * >(
        server->chooseConfig( configParams ));
    if( !config )
    {
        LBERROR << "No matching config on server" << std::endl;
        disconnectServer( server );
        exitLocal();
        return EXIT_FAILURE;
    }

    FrameData& frameData = config->getFrameData();
    frameData.setup( _impl->_applicationParameters, _impl->_rendererParameters );
    frameData.getVolumeSettings()->setURI( _impl->_applicationParameters.dataFileName);

    // 3. init config
    lunchbox::Clock clock;
    if( !config->init( argc, argv ))
    {
        server->releaseConfig( config );
        disconnectServer( server );
        exitLocal();
        return EXIT_FAILURE;
    }
    LBLOG( LOG_STATS ) << "Config init took " << clock.getTimef() << " ms"
                       << std::endl;

    // 4. run main loop
    uint32_t maxFrames = _impl->_applicationParameters.maxFrames;
    frameData.getCameraSettings()->setDefaultCameraPosition(
        _impl->_applicationParameters.cameraPosition );
    frameData.getCameraSettings()->setDefaultCameraLookAt(
        _impl->_applicationParameters.cameraLookAt );

    clock.reset();
    while( config->isRunning() && maxFrames-- )
    {
        if( !config->frame()) // If not valid, reset maxFrames
            maxFrames++;

        if( _impl->_idleFunc )
            _impl->_idleFunc(); // order is important to latency

        while( !config->needRedraw( )) // wait for an event requiring redraw
        {
            if( hasCommands( )) // execute non-critical pending commands
            {
                processCommand();
                config->handleEvents(); // non-blocking
            }
            else  // no pending commands, block on user event
            {
                // Poll ZeroEq subscribers at least every 100 ms in handleEvents
                const eq::EventICommand& event = config->getNextEvent( 100 );
                if( event.isValid( ))
                    config->handleEvent( event );
                config->handleEvents(); // non-blocking
            }
        }
        config->handleEvents(); // process all pending events
    }

    const uint32_t frame = config->finishAllFrames();
    const float    time  = clock.getTimef();
    LBLOG( LOG_STATS ) << "Rendering took " << time << " ms (" << frame
                       << " frames @ " << ( frame / time * 1000.f) << " FPS)"
                       << std::endl;

    // 5. exit config
    clock.reset();
    config->exit();
    LBLOG( LOG_STATS ) << "Exit took " << clock.getTimef() << " ms" <<std::endl;

    // 6. cleanup and exit
    server->releaseConfig( config );
    if( !disconnectServer( server ))
        LBERROR << "Client::disconnectServer failed" << std::endl;
    server = 0;
    exitLocal();

    return EXIT_SUCCESS;
}

void Client::clientLoop()
{
    do
    {
        LBINFO << "Entered client loop" << std::endl;
        const uint32_t timeout = 100; // Run idle function
                                      // every 100ms
        while( isRunning( ))
        {
            if( _impl->_idleFunc )
                _impl->_idleFunc(); // order is important for latency

            processCommand( timeout );
        }

    }
    while( _impl->_applicationParameters.isResident ); // execute at least one config run
}

}

