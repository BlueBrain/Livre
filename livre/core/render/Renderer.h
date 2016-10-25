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

#ifndef _Renderer_h_
#define _Renderer_h_

#include <livre/core/api.h>
#include <livre/core/types.h>

namespace livre
{

static const uint32_t RENDER_BEGIN = 1u;
static const uint32_t RENDER_FRAME = 1u << 1;
static const uint32_t RENDER_END   = 1u << 2;
static const uint32_t RENDER_ALL   = RENDER_BEGIN | RENDER_FRAME | RENDER_END;

/** The Renderer class is the base class for renderers. */
class Renderer
{
public:

    /**
     * Constructor
     * @param name name of the renderer
     */
    LIVRECORE_API Renderer( const std::string& name );
    LIVRECORE_API ~Renderer();

    /**
     * Renders the list of render bricks for a given frustum.
     * @param renderInputs necessary information for rendering
     * @param renderData hold the list of cached objects for rendering. Renderer
     * implicitly knows the type of the cache object and does the dynamic_casting
     * accordingly
     * @param renderStages The bitwise flags for enabling/disabling certain stages of
     * rendering. i.e. With different settings, multipass rendering can be performed in
     * the same frame.
     */
    void render( const RenderInputs& renderInputs,
                 const ConstCacheObjects& renderData,
                 const uint32_t renderStages = RENDER_ALL );

private:

    struct Impl;
    std::unique_ptr< Impl > _impl;
};

}
#endif // _Renderer_h_
