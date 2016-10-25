/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include "Renderer.h"
#include "RendererPlugin.h"
#include "RenderPipeline.h"

#include <livre/core/render/Frustum.h>
#include <livre/core/data/LODNode.h>

#include <lunchbox/pluginFactory.h>

namespace livre
{

struct Renderer::Impl
{
public:
    typedef lunchbox::PluginFactory< RendererPlugin, std::string > PluginFactory;

    Impl( const std::string& name )
        : plugin( PluginFactory::getInstance().create( name ))
    {}

    void render( const RenderInputs& renderInputs,
                 const ConstCacheObjects& renderData,
                 const uint32_t renderStages /*= RENDER_ALL*/ )
    {
       if( renderStages & RENDER_BEGIN )
           plugin->preRender( renderInputs, renderData );

       if( renderStages & RENDER_FRAME )
           plugin->render( renderInputs, renderData );

       if( renderStages & RENDER_END )
           plugin->postRender( renderInputs, renderData );
    }

    std::unique_ptr< RendererPlugin > plugin;
};

Renderer::Renderer( const std::string& name )
    : _impl( new Renderer::Impl( name ))
{}

Renderer::~Renderer()
{}

void Renderer::render( const RenderInputs& renderInputs,
                       const ConstCacheObjects& renderData,
                       const uint32_t renderStages /*= RENDER_ALL*/ )
{
    _impl->render( renderInputs, renderData, renderStages );
}

}
