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
     * @param histogramCache the histogram cache
     * @param glContext the gl context that will be shared
     */
    RenderPipeline( TextureCache& textureCache,
                    HistogramCache& histogramCache,
                    ConstGLContextPtr glContext );

    ~RenderPipeline();

    /**
     * Renders a frame using the given frustum and view
     * @param vrParams rendering parameters
     * @param frameInfo frustum and frame id
     * @param dataRange range of the data
     * @param pixelViewPort the view port
     * @param redrawFilter executed on data update
     * @param sendHistogramFilter executed on histogram computation
     * @param renderer the rendering algorithm
     * @param availability the number of available and not available nodes are written
     */
    void render( const VolumeRendererParameters& vrParams,
                 const FrameInfo& frameInfo,
                 const Range& dataRange,
                 const PixelViewport& pixelViewPort,
                 const Viewport& viewport,
                 const PipeFilter& redrawFilter,
                 const PipeFilter& sendHistogramFilter,
                 Renderer& renderer,
                 NodeAvailability& avaibility ) const;
private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif

