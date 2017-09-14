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
#include <livre/data/DataSource.h>

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
    NodeFactory() {}
    virtual eq::Config* createConfig(eq::ServerPtr parent)
    {
        return new livre::Config(parent);
    }

    virtual eq::Node* createNode(eq::Config* parent)
    {
        return new livre::Node(parent);
    }

    virtual eq::Pipe* createPipe(eq::Node* parent)
    {
        return new livre::Pipe(parent);
    }

    virtual eq::Window* createWindow(eq::Pipe* parent)
    {
        return new livre::Window(parent);
    }

    virtual eq::Channel* createChannel(eq::Window* parent)
    {
        return new livre::Channel(parent);
    }
};
}

namespace livre
{
struct Engine::Impl
{
    Impl(int argc, char** argv)
    {
        livre::initErrors();

        if (!eq::init(argc, argv, &nodeFactory))
        {
            eq::exit();
            livre::exitErrors();
            LBTHROW(std::runtime_error("Equalizer init failed"));
        }

        DataSource::loadPlugins();
        client = new livre::Client(argc, argv);
    }

    ~Impl()
    {
        client->exit();
        client = nullptr;
        eq::exit();
        livre::exitErrors();
    }

    NodeFactory nodeFactory;
    lunchbox::RefPtr<livre::Client> client;
};

Engine::Engine(int argc, char** argv)
    : _impl(new Engine::Impl(argc, argv))
{
}

Engine::~Engine()
{
}

std::string Engine::getHelp()
{
    DataSource::loadPlugins(); // needed to complete --volume
    return ApplicationParameters().getHelp() +
           VolumeRendererParameters().getHelp();
}

std::string Engine::getVersion()
{
    std::stringstream os;
    os << "Livre version " << livrecore::Version::getString() << std::endl;
    return os.str();
}

void Engine::run()
{
    _impl->client->run();
}

bool Engine::render(const eq::View::ScreenshotFunc& func)
{
    return _impl->client->render(func);
}

void Engine::resize(const Vector2ui& size)
{
    _impl->client->resize(size);
}

const Histogram& Engine::getHistogram() const
{
    return _impl->client->getHistogram();
}

FrameData& Engine::getFrameData()
{
    return _impl->client->getFrameData();
}

const VolumeInformation& Engine::getVolumeInformation() const
{
    return _impl->client->getVolumeInformation();
}

ApplicationParameters& Engine::getApplicationParameters()
{
    return _impl->client->getApplicationParameters();
}
}
