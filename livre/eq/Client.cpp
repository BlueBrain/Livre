
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
    Impl(const int argc, char* argv[])
        : applicationParameters(argc, argv)
        , rendererParameters(argc, argv)
    {
    }

    IdleFunc _idleFunc;
    eq::ServerPtr server;
    Config* config{nullptr};
    ApplicationParameters applicationParameters;
    VolumeRendererParameters rendererParameters;
};

Client::Client(const int argc, char* argv[])
    : _impl(new Client::Impl(argc, argv))
{
    if (_impl->applicationParameters.dataFileName.empty())
        _impl->applicationParameters.dataFileName = "mem:///#4096,4096,4096,40";

    if (!initLocal(argc, argv))
        LBTHROW(std::runtime_error("Can't init client"));

    _impl->server = new eq::Server;
    if (!connectServer(_impl->server))
    {
        exitLocal();
        LBTHROW(std::runtime_error("Can't open server"));
    }

    eq::fabric::ConfigParams configParams;
    _impl->config =
        static_cast<Config*>(_impl->server->chooseConfig(configParams));
    if (!_impl->config)
        LBTHROW(std::runtime_error("No matching config on server"));

    // framedata setup gets volume URI from application params which is
    // needed in Node::configInit() to load volume
    FrameData& frameData = _impl->config->getFrameData();
    frameData.setup(_impl->applicationParameters, _impl->rendererParameters);

    if (!_impl->config->init(argc, argv))
    {
        _impl->server->releaseConfig(_impl->config);
        _impl->config = nullptr;
        LBTHROW(std::runtime_error("Config init failed"));
    }
}

Client::~Client()
{
}

void Client::run()
{
    uint32_t maxFrames = _impl->applicationParameters.maxFrames;
    while (_impl->config->isRunning() && maxFrames--)
    {
        if (!_impl->config->frame()) // If not valid, reset maxFrames
            maxFrames++;

        if (getIdleFunction())
            getIdleFunction()(); // order is important to latency

        // wait for an event requiring redraw
        while (!_impl->config->needRedraw())
        {
            if (hasCommands())
            {
                processCommand();
                _impl->config->handleEvents(); // non-blocking
            }
            else // no pending commands, block on user event
            {
                // Poll ZeroEq subscribers at least every 100 ms in handleEvents
                const eq::EventICommand& event =
                    _impl->config->getNextEvent(100);
                if (event.isValid())
                    _impl->config->handleEvent(event);
                _impl->config->handleEvents();        // non-blocking
                _impl->config->handleNetworkEvents(); // non-blocking
            }
        }
        _impl->config->handleEvents();        // process all pending events
        _impl->config->handleNetworkEvents(); // ...all ZeroEQ events
    }
}

bool Client::render(const eq::View::ScreenshotFunc& func)
{
    eq::View* view{nullptr};
    auto layout = getActiveLayout();
    if (layout)
    {
        auto& views = layout->getViews();
        if (!views.empty())
        {
            view = views.front();
            view->enableScreenshot(eq::Frame::Buffer::color, func);
        }
    }

    _impl->config->frame();
    if (getIdleFunction())
        getIdleFunction()(); // order is important to latency
    _impl->config->handleEvents();

    if (view)
        view->disableScreenshot();

    return _impl->config->needRedraw();
}

void Client::exit()
{
    if (_impl->config)
    {
        _impl->config->exit();
        _impl->server->releaseConfig(_impl->config);
    }
    if (!disconnectServer(_impl->server))
        LBERROR << "Client::disconnectServer failed" << std::endl;
    _impl->server = 0;
    exitLocal();

    LBASSERTINFO(getRefCount() == 1, "Client still referenced by "
                                         << getRefCount() - 1);
}

void Client::resize(const Vector2ui& size)
{
    _impl->config->finishAllFrames();
    auto layout = getActiveLayout();
    if (layout)
        layout->setPixelViewport(
            eq::PixelViewport{0, 0, int32_t(size.x()), int32_t(size.y())});
}

void Client::setIdleFunction(const IdleFunc& idleFunc)
{
    _impl->_idleFunc = idleFunc;
}

const IdleFunc& Client::getIdleFunction() const
{
    return _impl->_idleFunc;
}

bool Client::initLocal(const int argc, char** argv)
{
    addActiveLayout("Simple"); // prefer single GPU layout by default
    return eq::Client::initLocal(argc, argv);
}

void Client::clientLoop()
{
    LBINFO << "Entered client loop" << std::endl;
    const uint32_t timeout = 100; // Run idle function every 100ms
    while (isRunning())
    {
        if (_impl->_idleFunc)
            _impl->_idleFunc(); // order is important for latency

        processCommand(timeout);
    }
}

eq::Layout* Client::getActiveLayout()
{
    if (!_impl->config)
        return nullptr;
    auto& canvases = _impl->config->getCanvases();
    if (!canvases.empty())
        return canvases.front()->getActiveLayout();
    return nullptr;
}

const Histogram& Client::getHistogram() const
{
    return _impl->config->getHistogram();
}

const FrameData& Client::getFrameData() const
{
    return _impl->config->getFrameData();
}

FrameData& Client::getFrameData()
{
    return _impl->config->getFrameData();
}

const VolumeInformation& Client::getVolumeInformation() const
{
    return _impl->config->getVolumeInformation();
}

ApplicationParameters& Client::getApplicationParameters()
{
    return _impl->applicationParameters;
}
}
