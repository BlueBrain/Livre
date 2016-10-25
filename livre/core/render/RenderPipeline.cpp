/* Copyright (c) 2011-2016  Ahmet Bilgili <ahmetbilgili@gmail.com>
 *
 * This file is part of Livre <https://github.com/bilgili/Livre>
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

#include "RenderPipeline.h"
#include "RenderPipelinePlugin.h"
#include "Renderer.h"

#include <livre/core/render/Frustum.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/cache/CacheObject.h>
#include <livre/core/version.h>

#include <lunchbox/pluginFactory.h>

namespace livre
{

namespace
{
    lunchbox::DSOs _plugins;
}

struct RenderPipeline::Impl
{
public:
    typedef lunchbox::PluginFactory< RenderPipelinePlugin, std::string > PluginFactory;

    Impl( const std::string& name )
        : plugin( PluginFactory::getInstance().create( name ))
        , renderer( new Renderer( name ))
    {}

    RenderStatistics render( const RenderInputs& renderInputs )
    {
         return plugin->render( *renderer, renderInputs );
    }

    std::unique_ptr< RenderPipelinePlugin > plugin;
    std::unique_ptr< Renderer > renderer;
};

RenderPipeline::RenderPipeline( const std::string& name )
    : _impl( new RenderPipeline::Impl( name ))
{}

RenderPipeline::~RenderPipeline()
{}

void RenderPipeline::loadPlugins()
{
    unloadPlugins();
    _plugins = RenderPipeline::Impl::PluginFactory::getInstance().load(
        LIVRECORE_VERSION_ABI, lunchbox::getLibraryPaths(), "Livre.*Pipeline" );
}

void RenderPipeline::unloadPlugins()
{
    for( lunchbox::DSO* plugin: _plugins )
        RenderPipeline::Impl::PluginFactory::getInstance().unload( plugin );
}

RenderStatistics RenderPipeline::render( const RenderInputs& renderInputs )
{
    return _impl->render( renderInputs );
}

const Renderer& RenderPipeline::getRenderer() const
{
    return *_impl->renderer;
}
}
