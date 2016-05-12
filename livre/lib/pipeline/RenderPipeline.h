/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _RenderPipeline_h_
#define _RenderPipeline_h_

#include <livre/lib/types.h>
#include <livre/core/mathTypes.h>

namespace livre
{

/**
 * RenderPipeline generates and executes the rendering pipeline every frame
 */
class RenderPipeline
{

public:

    /**
     * Constructor
     * @param textureCache the texture cache
     * @param nComputeThreads number of compute threads
     * @param nUploadThreads number of upload threads
     * @param glContext the gl context that will be shared
     */
    RenderPipeline( TextureCache& textureCache,
                    const size_t nComputeThreads,
                    const size_t nUploadThreads,
                    ConstGLContextPtr glContext );

    ~RenderPipeline();

    /**
     * Renders a frame using the given frustum and view
     * @param vrParams rendering parameters
     * @param frameInfo frustum and frame id
     * @param dataRange range of the data
     * @param pixelViewPort the view port
     * @param redrawFilter executed on data update
     * @param renderer the rendering algorithm
     * @param nAvailable number of available texture blocks
     * @param nNotAvailable number of not available texture blocks
     */
    void render( const VolumeRendererParameters& vrParams,
                 const FrameInfo& frameInfo,
                 const Range& dataRange,
                 const PixelViewport& pixelViewPort,
                 const PipeFilter& redrawFilter,
                 Renderer& renderer,
                 size_t& nAvailable,
                 size_t& nNotAvailable ) const;
private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif

