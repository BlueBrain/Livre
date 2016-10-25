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


#ifndef _RenderPipelinePlugin_h_
#define _RenderPipelinePlugin_h_

#include <livre/core/api.h>
#include <livre/core/types.h>

#include <lunchbox/plugin.h>

namespace livre
{

class RenderPipelinePlugin
{
public:

    /** Needed by the PluginRegisterer. */
    typedef RenderPipelinePlugin PluginT;

    /** Needed by the PluginRegisterer. */
    typedef std::string InitDataT;

    LIVRECORE_API RenderPipelinePlugin( const std::string& name LB_UNUSED ) {}
    LIVRECORE_API virtual ~RenderPipelinePlugin() {}

    virtual RenderStatistics render( Renderer& renderer,
                                     const RenderInputs& renderInputs ) = 0;
};
}
#endif // _RenderPipelinePlugin_h_

