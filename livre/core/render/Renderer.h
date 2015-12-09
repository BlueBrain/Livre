/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#ifndef _Renderer_h_
#define _Renderer_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/render/TexturePool.h>
#include <livre/core/render/TextureState.h>

namespace livre
{

/**
 * The Renderer class is the base class for renderers.
 */
class Renderer
{
public:

    /**
     * Renders the list of render bricks for a given frustum.
     * @param glWidget Widget to render to.
     * @param view The viewing information on the widget.
     * @param brickList The list of render bricks.
     */
    LIVRECORE_API void render( const GLWidget& glWidget,
                               const View& view,
                               const RenderBricks& brickList );

    /**
     * @return The OpenGL GPU internal format of the texture data.
     */
    LIVRECORE_API int32_t getInternalFormat() const;

    /**
     * @return The OpenGL format of the texture data kept in memory. Channel count etc.
     */
    LIVRECORE_API uint32_t getGPUDataType() const;

    /**
     * @return The OpenGL data type of the texture data.
     */
    LIVRECORE_API uint32_t getFormat() const;

protected:
    /**
     * Orders the render bricks front to back ( default ).
     * @param bricks The list of bricks to be ordered.
     * @param frustum The frustum to order the bricks for.
     */
    LIVRECORE_API virtual void order_( RenderBricks& bricks, const Frustum& frustum ) const;

    /**
     * Is called on start of each rendering.
     * @param glWidget The widget that has the GL context.
     * @param view The definition of view on context
     * @param brickList The list of oredered bricks.
     */
    virtual void onFrameStart_( const GLWidget& glWidget LB_UNUSED,
                                const View& view LB_UNUSED,
                                const RenderBricks& brickList LB_UNUSED ) { }

    /**
     * Is called on start of each render. Default is front to back rendering.
     * @param glWidget The widget that has the GL context.
     * @param view The definition of view on context
     * @param frustum The frustum to order the bricks for.
     * @param brickList The list of ordered bricks.
    */
    LIVRECORE_API virtual void onFrameRender_( const GLWidget& glWidget,
                                               const View& view,
                                               const RenderBricks& brickList );

    /**
     * Is called on end of each rendering.
     * @param glWidget The widget that has the GL context.
     * @param view The definition of view on context
     * @param brickList The list of ordered bricks.
     */
    virtual void onFrameEnd_( const GLWidget& glWidget LB_UNUSED,
                              const View& view LB_UNUSED,
                              const RenderBricks& brickList LB_UNUSED ) { }

    /**
     * Should be implemented by the derived renderer to render a render brick.
     * @param glWidget The widget that has the GL context.
     * @param view The definition of view on context
     * @param renderBrick The list of ordered bricks.
     */
    virtual void renderBrick_(  const GLWidget& glWidget,
                                const View& view,
                                const RenderBrick& renderBrick ) = 0;

    /**
     * @param nComponents The number of components in the rendering.
     * @param gpuDataType The OpenGL data type.
     * @param internalFormat The OpenGL internal format.
     */
    LIVRECORE_API Renderer( const uint32_t nComponents,
                            const uint32_t gpuDataType,
                            const int32_t internalFormat );

    LIVRECORE_API virtual ~Renderer();

private:
    uint32_t gpuDataType_;
    int32_t internalFormat_;
    uint32_t format_;

};

}
#endif // _Renderer_h_
