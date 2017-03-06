/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
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

#include <livre/lib/pipeline/DataUploadFilter.h>
#include <livre/lib/pipeline/HistogramFilter.h>
#include <livre/lib/pipeline/RenderFilter.h>
#include <livre/lib/pipeline/RenderPipeline.h>
#include <livre/lib/pipeline/RenderingSetGeneratorFilter.h>
#include <livre/lib/pipeline/VisibleSetGeneratorFilter.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/pipeline/Pipeline.h>
#include <livre/core/pipeline/SimpleExecutor.h>

#include <livre/core/render/Renderer.h>
#include <livre/core/render/TexturePool.h>

#include <boost/progress.hpp>

namespace livre
{
namespace
{
const size_t nRenderThreads = 2;
const size_t nUploadThreads = 1;
const size_t nComputeThreads = 2;
}

struct RenderPipeline::Impl
{
    Impl(DataSource& dataSource, Caches& caches, TexturePool& texturePool,
         ConstGLContextPtr glContext)
        : _dataSource(dataSource)
        , _dataCache(caches.dataCache)
        , _textureCache(caches.textureCache)
        , _histogramCache(caches.histogramCache)
        , _texturePool(texturePool)
        , _renderExecutor(nRenderThreads, glContext)
        , _computeExecutor(nComputeThreads, glContext)
        , _uploadExecutor(nUploadThreads, glContext)
    {
    }

    void setupVisibleGeneratorFilter(PipeFilter& visibleSetGenerator,
                                     const RenderParams& renderParams) const
    {
        visibleSetGenerator.getPromise("Frustum").set(
            renderParams.frameInfo.frustum);
        visibleSetGenerator.getPromise("Frame").set(
            renderParams.frameInfo.timeStep);
        visibleSetGenerator.getPromise("DataRange")
            .set(renderParams.renderDataRange);
        visibleSetGenerator.getPromise("Params").set(renderParams.vrParams);
        visibleSetGenerator.getPromise("Viewport")
            .set(renderParams.pixelViewPort);
        visibleSetGenerator.getPromise("ClipPlanes")
            .set(renderParams.clipPlanes);
    }

    void setupRenderFilter(PipeFilter& renderFilter,
                           const RenderParams& renderParams,
                           const uint32_t renderStages) const
    {
        renderFilter.getPromise("Frustum").set(renderParams.frameInfo.frustum);
        renderFilter.getPromise("Viewport").set(renderParams.pixelViewPort);
        renderFilter.getPromise("ClipPlanes").set(renderParams.clipPlanes);
        renderFilter.getPromise("RenderStages").set(renderStages);
    }

    void renderSync(const RenderParams& renderParams,
                    PipeFilter& sendHistogramFilter, Renderer& renderer,
                    NodeAvailability& availability) const
    {
        PipeFilterT<VisibleSetGeneratorFilter> visibleSetGenerator(
            "VisibleSetGenerator", _dataSource);
        setupVisibleGeneratorFilter(visibleSetGenerator, renderParams);
        visibleSetGenerator.execute();

        const livre::UniqueFutureMap portFutures(
            visibleSetGenerator.getPostconditions());
        const auto& nodeIds =
            renderer.order(portFutures.get<NodeIds>("VisibleNodes"),
                           renderParams.frameInfo.frustum);

        const VolumeInformation& volInfo = _dataSource.getVolumeInfo();
        const size_t blockMemSize = volInfo.maximumBlockSize.product() *
                                    volInfo.getBytesPerVoxel() *
                                    volInfo.compCount;

        const uint32_t maxNodesPerPass =
            renderParams.vrParams.getMaxGpuCacheMemory() * LB_1MB /
            blockMemSize;

        const uint32_t numberOfPasses =
            std::ceil((float)nodeIds.size() / (float)maxNodesPerPass);

        std::unique_ptr<boost::progress_display> showProgress;
        if (numberOfPasses > 1)
        {
            LBINFO << "Multipass rendering. Number of passes: "
                   << numberOfPasses << std::endl;
            showProgress.reset(new boost::progress_display(numberOfPasses));
        }

        sendHistogramFilter.getPromise("RelativeViewport")
            .set(renderParams.viewport);
        sendHistogramFilter.getPromise("Id").set(
            renderParams.frameInfo.frameId);

        for (uint32_t i = 0; i < numberOfPasses; ++i)
        {
            uint32_t renderStages = RENDER_FRAME;
            if (i == 0)
                renderStages |= RENDER_BEGIN;

            if (i == numberOfPasses - 1u)
                renderStages |= RENDER_END;

            const uint32_t startIndex = i * maxNodesPerPass;
            const uint32_t endIndex = (i + 1) * maxNodesPerPass;
            const NodeIds nodesPerPass(nodeIds.begin() + startIndex,
                                       endIndex > nodeIds.size()
                                           ? nodeIds.end()
                                           : nodeIds.begin() + endIndex);

            createAndExecuteSyncPass(nodesPerPass, renderParams,
                                     sendHistogramFilter, renderer,
                                     renderStages);
            if (numberOfPasses > 1)
                ++(*showProgress);
        }

        const UniqueFutureMap futures(visibleSetGenerator.getPostconditions());
        availability.nAvailable = futures.get<NodeIds>("VisibleNodes").size();
        availability.nNotAvailable = 0;
    }

    void renderAsync(const RenderParams& renderParams,
                     PipeFilter& sendHistogramFilter, Renderer& renderer,
                     NodeAvailability& availability,
                     PipeFilter& redrawFilter) const
    {
        PipeFilterT<HistogramFilter> histogramFilter("HistogramFilter",
                                                     _histogramCache,
                                                     _dataCache, _dataSource);
        histogramFilter.getPromise("Frustum").set(
            renderParams.frameInfo.frustum);
        histogramFilter.connect("Histogram", sendHistogramFilter, "Histogram");
        histogramFilter.getPromise("RelativeViewport")
            .set(renderParams.viewport);
        histogramFilter.getPromise("DataSourceRange")
            .set(renderParams.dataSourceRange);
        sendHistogramFilter.getPromise("RelativeViewport")
            .set(renderParams.viewport);
        sendHistogramFilter.getPromise("Id").set(
            renderParams.frameInfo.frameId);

        Pipeline renderPipeline;
        Pipeline uploadPipeline;

        PipeFilterT<RenderFilter> renderFilter("RenderFilter", _dataSource,
                                               renderer);

        PipeFilter visibleSetGenerator =
            renderPipeline.add<VisibleSetGeneratorFilter>("VisibleSetGenerator",
                                                          _dataSource);
        setupVisibleGeneratorFilter(visibleSetGenerator, renderParams);

        PipeFilter renderingSetGenerator =
            renderPipeline.add<RenderingSetGeneratorFilter>(
                "RenderingSetGenerator", _textureCache);

        visibleSetGenerator.connect("VisibleNodes", renderingSetGenerator,
                                    "VisibleNodes");
        renderingSetGenerator.connect("CacheObjects", renderFilter,
                                      "CacheObjects");
        renderingSetGenerator.connect("CacheObjects", histogramFilter,
                                      "CacheObjects");
        renderingSetGenerator.connect("RenderingDone", redrawFilter,
                                      "RenderingDone");

        for (size_t i = 0; i < nUploadThreads; ++i)
        {
            std::stringstream name;
            name << "DataUploader" << i;
            PipeFilter uploader =
                uploadPipeline.add<DataUploadFilter>(name.str(), i,
                                                     nUploadThreads, _dataCache,
                                                     _textureCache, _dataSource,
                                                     _texturePool);

            visibleSetGenerator.connect("VisibleNodes", uploader,
                                        "VisibleNodes");
            visibleSetGenerator.connect("Params", uploader, "Params");
            uploader.connect("CacheObjects", redrawFilter, "CacheObjects");
        }

        setupRenderFilter(renderFilter, renderParams, RENDER_ALL);

        redrawFilter.schedule(_renderExecutor);
        renderPipeline.schedule(_renderExecutor);
        uploadPipeline.schedule(_uploadExecutor);
        sendHistogramFilter.schedule(_computeExecutor);
        histogramFilter.schedule(_computeExecutor);
        renderFilter.execute();

        const UniqueFutureMap futures(
            renderingSetGenerator.getPostconditions());
        availability = futures.get<NodeAvailability>("NodeAvailability");
    }

    void createAndExecuteSyncPass(NodeIds nodeIds,
                                  const RenderParams& renderParams,
                                  PipeFilter& sendHistogramFilter,
                                  Renderer& renderer,
                                  const uint32_t renderStages) const
    {
        PipeFilterT<HistogramFilter> histogramFilter("HistogramFilter",
                                                     _histogramCache,
                                                     _dataCache, _dataSource);
        histogramFilter.getPromise("Frustum").set(
            renderParams.frameInfo.frustum);
        histogramFilter.connect("Histogram", sendHistogramFilter, "Histogram");
        histogramFilter.getPromise("RelativeViewport")
            .set(renderParams.viewport);
        histogramFilter.getPromise("DataSourceRange")
            .set(renderParams.dataSourceRange);

        Pipeline renderPipeline;
        Pipeline uploadPipeline;

        PipeFilterT<RenderFilter> renderFilter("RenderFilter", _dataSource,
                                               renderer);
        setupRenderFilter(renderFilter, renderParams, renderStages);

        for (size_t i = 0; i < nUploadThreads; ++i)
        {
            std::stringstream name;
            name << "DataUploader" << i;
            PipeFilter uploader =
                uploadPipeline.add<DataUploadFilter>(name.str(), i,
                                                     nUploadThreads, _dataCache,
                                                     _textureCache, _dataSource,
                                                     _texturePool);

            uploader.getPromise("VisibleNodes").set(nodeIds);
            uploader.getPromise("Params").set(renderParams.vrParams);
            uploader.connect("CacheObjects", renderFilter, "CacheObjects");
            uploader.connect("CacheObjects", histogramFilter, "CacheObjects");
        }

        renderPipeline.schedule(_renderExecutor);
        uploadPipeline.schedule(_uploadExecutor);
        sendHistogramFilter.schedule(_computeExecutor);
        histogramFilter.schedule(_computeExecutor);
        renderFilter.execute();
    }

    void render(const RenderParams& renderParams, PipeFilter& redrawFilter,
                PipeFilter& sendHistogramFilter, Renderer& renderer,
                NodeAvailability& availability) const
    {
        if (renderParams.vrParams.getSynchronousMode())
            renderSync(renderParams, sendHistogramFilter, renderer,
                       availability);
        else
            renderAsync(renderParams, sendHistogramFilter, renderer,
                        availability, redrawFilter);
    }

    DataSource& _dataSource;
    Cache& _dataCache;
    Cache& _textureCache;
    Cache& _histogramCache;
    TexturePool& _texturePool;
    mutable SimpleExecutor _renderExecutor;
    mutable SimpleExecutor _computeExecutor;
    mutable SimpleExecutor _uploadExecutor;
};

RenderPipeline::RenderPipeline(DataSource& dataSource, Caches& caches,
                               TexturePool& texturePool,
                               ConstGLContextPtr glContext)
    : _impl(
          new RenderPipeline::Impl(dataSource, caches, texturePool, glContext))
{
}

RenderPipeline::~RenderPipeline()
{
}

void RenderPipeline::render(const RenderParams& renderParams,
                            PipeFilter redrawFilter,
                            PipeFilter sendHistogramFilter, Renderer& renderer,
                            NodeAvailability& avaibility) const
{
    _impl->render(renderParams, redrawFilter, sendHistogramFilter, renderer,
                  avaibility);
}
}
