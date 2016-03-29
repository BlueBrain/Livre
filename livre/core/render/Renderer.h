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
     * @param frustum is used for rendering the bricks according to view point.
     * @param view The pixel area.
     * @param bricks The list of render bricks.
     */
    LIVRECORE_API void render( const Frustum& frustum,
                               const PixelViewport& view,
                               const RenderBricks& bricks );

protected:

    /**
     * Orders the render bricks front to back ( default ).
     * @param bricksSrc The list of bricks to be ordered.
     * @param frustum is used to order the bricks according to view point.
     * @return the list of ordered bricks.
     */
    LIVRECORE_API virtual RenderBricks _order( const RenderBricks& bricks,
                                               const Frustum& frustum ) const;

    /**
     * Is called on start of each rendering.
     * @param frustum is used for rendering the bricks according to view point.
     * @param view The pixel area.
     * @param orderedBricks is the list of ordered bricks.
     */
    virtual void _onFrameStart( const Frustum& frustum LB_UNUSED,
                                const PixelViewport& view LB_UNUSED,
                                const RenderBricks& orderedBricks LB_UNUSED ) { }

    /**
     * Is called on start of each render. Default is front to back rendering.
     * @param frustum is used for rendering the bricks according to view point.
     * @param view The pixel area.
     * @param orderedBricks is the list of ordered bricks.
    */
    LIVRECORE_API virtual void _onFrameRender( const Frustum& frustum,
                                               const PixelViewport& view,
                                               const RenderBricks& orderedBricks );

    /**
     * Is called on end of each rendering.
     * @param frustum is used for rendering the bricks according to view point.
     * @param view The pixel area.
     * @param orderedBricks is the list of ordered bricks.
     */
    virtual void _onFrameEnd( const Frustum& frustum LB_UNUSED,
                              const PixelViewport& view LB_UNUSED,
                              const RenderBricks& orderedBricks LB_UNUSED ) { }

    /**
     * Should be implemented by the derived renderer to render a render brick.
     * @param frustum is used for rendering the bricks according to view point.
     * @param view The pixel area.
     * @param renderBrick is rendered brick
     */
    virtual void _renderBrick(  const Frustum& frustum,
                                const PixelViewport& view,
                                const RenderBrick& renderBrick ) = 0;

    LIVRECORE_API virtual ~Renderer();
};

}
#endif // _Renderer_h_
