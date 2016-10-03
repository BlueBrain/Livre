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

#ifndef _RenderPipeline_h_
#define _RenderPipeline_h_

#include <livre/lib/configuration/VolumeRendererParameters.h>
#include <livre/lib/types.h>

#include <livre/core/render/ClipPlanes.h>
#include <livre/core/render/FrameInfo.h>

namespace livre
{

/** The Caches struct */
struct Caches
{
    Cache& dataCache;
    Cache& textureCache;
    Cache& histogramCache;
};

/** Parameters for rendering */
struct RenderParams
{
    VolumeRendererParameters vrParams;
    FrameInfo frameInfo;
    Range renderDataRange;
    Vector2f dataSourceRange;
    PixelViewport pixelViewPort;
    Viewport viewport;
    ClipPlanes clipPlanes;
};

/**
 * RenderPipeline generates and executes the rendering pipeline every frame
 */
class RenderPipeline
{

public:

    /**
     * Constructor
     * @param caches the data, texture and histogram cache
     * @param dataSource the data source
     * @param texturePool the pool for textures
     * @param glContext the gl context that will be shared
     */
    RenderPipeline( DataSource& dataSource,
                    Caches& caches,
                    TexturePool& texturePool,
                    ConstGLContextPtr glContext );

    ~RenderPipeline();

    /**
     * Renders a frame using the given frustum and view
     * @param renderParams parameters for rendering
     * @param redrawFilter executed on data update
     * @param sendHistogramFilter executed on histogram computation
     * @param renderer the rendering algorithm
     * @param availability the number of available and not available nodes are written
     */
    void render( const RenderParams& renderParams,
                 PipeFilter redrawFilter,
                 PipeFilter sendHistogramFilter,
                 Renderer& renderer,
                 NodeAvailability& avaibility ) const;
private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif
