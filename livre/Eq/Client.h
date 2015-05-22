/* Copyright (c) 2006-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          David Steiner   <steiner@ifi.uzh.ch>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
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

#ifndef _Client_h_
#define _Client_h_

#include <eq/client.h>

#include <livre/Eq/types.h>
#include <livre/Lib/Configuration/ApplicationParameters.h>
#include <livre/Lib/Configuration/EFPrefetchAlgorithmParameters.h>
#include <livre/Lib/Configuration/VolumeRendererParameters.h>
#ifdef LIVRE_USE_RESTBRIDGE
#  include <livre/Lib/Configuration/RESTParameters.h>
#endif

namespace livre
{

enum LogTopics
{
    LOG_STATS = lunchbox::LOG_CUSTOM      // 65536
};

/**
 * The client application class.
 */
class Client : public eq::Client
{
public:

    Client();
    virtual ~Client();

    /**
     * @return Help string
     */
    static std::string getHelp();

    /**
     * @return Version string
     */
    static std::string getVersion();

    /** @override eq::Client::initLocal() */
    bool initLocal( const int argc, char** argv ) override;

    /**
     * run Starts the client.
     * @param argc Argument count.
     * @param argv Argument list.
     * @return The exit code.
     */
    int32_t run();

protected:

    /**
     * Infinite client loop.
     */
    virtual void clientLoop();

private:
    ApplicationParameters _applicationParameters;
    VolumeRendererParameters _rendererParameters;
    EFPrefetchAlgorithmParameters _prefetchParameters;
#ifdef LIVRE_USE_RESTBRIDGE
    RESTParameters _restParameters;
#endif

    /**
     * Parse command line arguments.
     * @param argc Argument count.
     * @param argv Argument list.
     * @return true if the application should continue to run, false otherwise.
     */
    bool _parseArguments( const int32_t argc, char** argv );
};

}
#endif // _Client_h_
