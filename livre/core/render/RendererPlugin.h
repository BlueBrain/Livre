/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                         Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _RendererPlugin_h_
#define _RendererPlugin_h_

#include <livre/core/api.h>
#include <livre/core/types.h>

#include <lunchbox/plugin.h>

namespace livre
{

/** The Renderer plugin class is the base class for render plugins. */
class RendererPlugin
{
public:

    /**
     * Constructor
     * @param name is the name of the renderer plugin
     */
    RendererPlugin( const std::string& name LB_UNUSED ) {}

    /** Needed by the PluginRegisterer. */
    typedef RendererPlugin PluginT;

    /** Needed by the PluginRegisterer. */
    typedef std::string InitDataT;

    /**
     * Is called on start of each rendering.
     * @param renderInputs necessary information for rendering
     * @param bricks is the list of bricks.
     */
    virtual void preRender( const RenderInputs& renderInputs LB_UNUSED,
                            const ConstCacheObjects& renderData LB_UNUSED ) {}

    /**
     * Is called on start of each render. Default is front to back rendering.
     * @param renderInputs necessary information for rendering
     * @param orderedBricks is the list of bricks.
    */
    virtual void render( const RenderInputs& renderInputs LB_UNUSED,
                         const ConstCacheObjects& renderData LB_UNUSED ) = 0;

    /**
     * Is called on end of each rendering.
     * @param renderInputs necessary information for rendering
     * @param orderedBricks is the list of bricks.
     */
    virtual void postRender( const RenderInputs& renderInputs LB_UNUSED,
                             const ConstCacheObjects& renderData LB_UNUSED ) {}

    virtual ~RendererPlugin() {}
};
}
#endif // _RendererPlugin_h_
