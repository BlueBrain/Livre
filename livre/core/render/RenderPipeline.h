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


#ifndef _RenderPipeline_h_
#define _RenderPipeline_h_

#include <livre/core/api.h>
#include <livre/core/types.h>

#include <livre/core/render/FrameInfo.h>

namespace livre
{

class RenderPipeline
{
public:

    /**
     * Constructor
     * @param name name of the renderer
     */
    LIVRECORE_API RenderPipeline( const std::string& name );
    LIVRECORE_API ~RenderPipeline();

    /** Load all plugin DSOs.*/
    LIVRECORE_API static void loadPlugins();

    /** Unload all plugin DSOs. */
    LIVRECORE_API static void unloadPlugins();

    LIVRECORE_API RenderStatistics render( const RenderInputs& renderInputs );
    LIVRECORE_API const Renderer& getRenderer() const;

private:
    struct Impl;
    std::unique_ptr< Impl > _impl;
};
}
#endif // _RenderPipeline_h_

