
/* Copyright (c) 2006-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
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

#include <eq/eq.h>

#include <livre/data/DataSource.h>

#include <livre/lib/configuration/ApplicationParameters.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>

#include <livre/eq/Client.h>
#include <livre/eq/Config.h>
#include <livre/eq/FrameData.h>

namespace livre
{
struct Client::Impl
{
    Impl(const bool isResident_)
        : isResident(isResident_)
    {
    }
    IdleFunc _idleFunc;
    eq::ServerPtr server;
    bool isResident = false;
};

Client::Client(const int argc, char** argv, const bool isResident)
    : _impl(new Client::Impl(isResident))
{
    if (!initLocal(argc, argv))
        LBTHROW(std::runtime_error("Can't init client"));

    _impl->server = new eq::Server;
    if (!connectServer(_impl->server))
    {
        exitLocal();
        LBTHROW(std::runtime_error("Can't open server"));
    }
}

Client::~Client()
{
}

Config* Client::chooseConfig()
{
    eq::fabric::ConfigParams configParams;
    auto config =
        static_cast<Config*>(_impl->server->chooseConfig(configParams));
    return config;
}

void Client::releaseConfig(Config* config)
{
    _impl->server->releaseConfig(config);
    if (!disconnectServer(_impl->server))
        LBERROR << "Client::disconnectServer failed" << std::endl;
    _impl->server = 0;
    exitLocal();

    LBASSERTINFO(getRefCount() == 1, "Client still referenced by "
                                         << getRefCount() - 1);
}

void Client::setIdleFunction(const IdleFunc& idleFunc)
{
    _impl->_idleFunc = idleFunc;
}

const IdleFunc& Client::getIdleFunction() const
{
    return _impl->_idleFunc;
}

bool Client::processCommands()
{
    if (!hasCommands())
        return false;

    processCommand();
    return true;
}

bool Client::initLocal(const int argc, char** argv)
{
    addActiveLayout("Simple"); // prefer single GPU layout by default
    return eq::Client::initLocal(argc, argv);
}

void Client::clientLoop()
{
    do
    {
        LBINFO << "Entered client loop" << std::endl;
        const uint32_t timeout = 100; // Run idle function every 100ms
        while (isRunning())
        {
            if (_impl->_idleFunc)
                _impl->_idleFunc(); // order is important for latency

            processCommand(timeout);
        }
    } while (_impl->isResident); // execute at least one config run
}
}
