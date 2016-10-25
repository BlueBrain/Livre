/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#ifndef _GLRayCastRenderer_h_
#define _GLRayCastRenderer_h_

#include <livre/lib/types.h>
#include <livre/core/render/RendererPlugin.h>

namespace livre
{
namespace
{
static const std::string rendererName = "giRaycaster";
}

/** The RayCastRenderer class implements a single-pass ray caster. */
class GLRaycastRenderer : public RendererPlugin
{
public:

    /**
     * Constructor
     * @param name is the name of the render plugin
     */
    GLRaycastRenderer( const std::string& name );
    ~GLRaycastRenderer();

    /** @return true if renderer handles the name */
    static bool handles( const std::string& name ) { return name == "gl"; }

protected:

    void preRender( const RenderInputs& renderInputs,
                    const ConstCacheObjects& renderData ) final;

    void render( const RenderInputs& renderInputs,
                 const ConstCacheObjects& renderData ) final;

    void postRender( const RenderInputs& renderInputs,
                     const ConstCacheObjects& renderData ) final;

    struct Impl;
    std::unique_ptr< Impl > _impl;
};

}

#endif // _RayCastRenderer_h_
