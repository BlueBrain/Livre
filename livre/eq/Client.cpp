
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

#include <livre/eq/Client.h>
#include <livre/eq/Config.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/settings/VolumeSettings.h>
#include <livre/eq/settings/CameraSettings.h>

namespace livre
{

Client::Client()
{
}

Client::~Client()
{
    VolumeDataSource::unloadPlugins();
}

bool Client::_parseArguments( const int32_t argc, char **argv )
{
    if( !_applicationParameters.initialize( argc, argv ) ||
        !_rendererParameters.initialize( argc, argv ))
    {
        LBERROR << "Error parsing command line arguments" << std::endl;
        return false;
    }

#ifdef LIVRE_USE_RESTBRIDGE
    if( !_restParameters.initialize( argc, argv ))
    {
        LBERROR << "Error parsing command line arguments" << std::endl;
        return false;
    }
#endif

    if( _applicationParameters.dataFileName.empty())
        _applicationParameters.dataFileName = "mem:///#4096,4096,4096,32";

    return true;
}

std::string Client::getHelp()
{
    VolumeRendererParameters vrParameters;
    ApplicationParameters applicationParameters;
#ifdef LIVRE_USE_RESTBRIDGE
    RESTParameters restParameters;
#endif

    Configuration conf;
    conf.addDescription( vrParameters.getConfiguration( ));
    conf.addDescription( applicationParameters.getConfiguration( ));
#ifdef LIVRE_USE_RESTBRIDGE
    conf.addDescription( restParameters.getConfiguration( ));
#endif

    std::stringstream os;
    os << conf;
    return os.str();
}

std::string Client::getVersion()
{
    std::stringstream os;
    os << "Livre version " << livrecore::Version::getString() << std::endl;
    return os.str();
}

bool Client::initLocal( const int argc, char** argv )
{
    if( !_parseArguments( argc, argv ))
        return false;
    VolumeDataSource::loadPlugins(); //initLocal on render clients never returns
    addActiveLayout( "Simple" ); // prefer single GPU layout by default
    return eq::Client::initLocal( argc, argv );
}

int Client::run()
{
    // 1. connect to server
    eq::ServerPtr server = new eq::Server;
    if( !connectServer( server ))
    {
        LBERROR << "Can't open server" << std::endl;
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
        return EXIT_FAILURE;
    }

    FrameData& frameData = config->getFrameData();
    frameData.setup( _applicationParameters,
                     _rendererParameters,
                     _restParameters );
    frameData.getVolumeSettings()->setURI( _applicationParameters.dataFileName);

    // 3. init config
    lunchbox::Clock clock;
    if( !config->init())
    {
        server->releaseConfig( config );
        disconnectServer( server );
        return EXIT_FAILURE;
    }
    LBLOG( LOG_STATS ) << "Config init took " << clock.getTimef() << " ms"
                       << std::endl;

    // 4. run main loop
    uint32_t maxFrames = _applicationParameters.maxFrames;
    frameData.getCameraSettings()->setDefaultCameraPosition(
        _applicationParameters.cameraPosition );
    frameData.getCameraSettings()->setDefaultCameraLookAt(
        _applicationParameters.cameraLookAt );

    clock.reset();
    while( config->isRunning() && maxFrames-- )
    {
        config->frame();
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

    return EXIT_SUCCESS;
}

void Client::clientLoop()
{
    do
    {
         eq::Client::clientLoop();
         LBINFO << "Configuration run successfully executed" << std::endl;
    }
    while( _applicationParameters.isResident ); // execute at least one config run
}
}
