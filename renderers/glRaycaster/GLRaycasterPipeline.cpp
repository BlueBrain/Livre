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

#include "GLRaycasterPipeline.h"
#include "GLRenderUploadFilter.h"

#include <livre/lib/pipeline/RenderingSetGeneratorFilter.h>
#include <livre/lib/pipeline/VisibleSetGeneratorFilter.h>
#include <livre/lib/pipeline/DataUploadFilter.h>
#include <livre/lib/pipeline/RenderFilter.h>
#include <livre/lib/pipeline/HistogramFilter.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/pipeline/SimpleExecutor.h>
#include <livre/core/pipeline/Pipeline.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/render/RenderInputs.h>
#include <livre/core/render/TexturePool.h>
#include <livre/core/render/Renderer.h>
#include <livre/core/render/GLContext.h>
#include <livre/core/settings/RenderSettings.h>

#include <lunchbox/pluginRegisterer.h>
#include <boost/progress.hpp>

#include <livre/core/version.h>

extern "C"
int LunchboxPluginGetVersion() { return LIVRECORE_VERSION_ABI; }

extern "C"
bool LunchboxPluginRegister() { return true; }

namespace livre
{
namespace
{
const size_t nRenderThreads = 2;
const size_t nUploadThreads = 2;
const size_t nComputeThreads = 2;
lunchbox::PluginRegisterer< GLRaycasterPipeline > registerer;
}

struct GLRaycasterPipeline::Impl
{
    Impl()
        : _renderExecutor( nRenderThreads, "Render Executor", GLContext::getCurrent()->clone( ))
        , _computeExecutor( nComputeThreads, "Compute Executor", GLContext::getCurrent()->clone( ))
        , _uploadExecutor( nUploadThreads, "Upload Executor", GLContext::getCurrent()->clone( ))
    {
    }

    void setupVisibleGeneratorFilter( PipeFilter& visibleSetGenerator,
                                      const RenderInputs& renderInputs ) const
    {
        visibleSetGenerator.getPromise( "Frustum" ).set( renderInputs.frameInfo.frustum );
        visibleSetGenerator.getPromise( "Frame" ).set( renderInputs.frameInfo.timeStep );
        visibleSetGenerator.getPromise( "DataRange" ).set( renderInputs.renderDataRange );
        visibleSetGenerator.getPromise( "Params" ).set( renderInputs.vrParameters );
        visibleSetGenerator.getPromise( "Viewport" ).set( renderInputs.pixelViewPort );
        visibleSetGenerator.getPromise( "ClipPlanes" ).set(
                    renderInputs.renderSettings.getClipPlanes( ));
    }

    // Sort helper function for sorting the textures with their distances to viewpoint
    struct DistanceOperator
    {
        explicit DistanceOperator( const DataSource& dataSource, const Frustum& frustum )
            : _frustum( frustum )
            , _dataSource( dataSource )
        { }

        bool operator()( const NodeId& rb1, const NodeId& rb2 )
        {
            const LODNode& lodNode1 = _dataSource.getNode( rb1 );
            const LODNode& lodNode2 = _dataSource.getNode( rb2 );

            const float distance1 = ( _frustum.getMVMatrix() *
                                      lodNode1.getWorldBox().getCenter() ).length();
            const float distance2 = ( _frustum.getMVMatrix() *
                                      lodNode2.getWorldBox().getCenter() ).length();
            return  distance1 < distance2;
        }
        const Frustum& _frustum;
        const DataSource& _dataSource;
    };

    void renderSync( RenderStatistics& statistics,
                     Renderer& renderer,
                     const RenderInputs& renderInputs )
    {

        PipeFilter sendHistogramFilter = renderInputs.filters.find( "SendHistogramFilter" )->second;
        PipeFilter preRenderFilter = renderInputs.filters.find( "PreRenderFilter" )->second;
        PipeFilterT< VisibleSetGeneratorFilter > visibleSetGenerator( "VisibleSetGenerator",
                                                                      renderInputs.dataSource );
        setupVisibleGeneratorFilter( visibleSetGenerator, renderInputs );
        visibleSetGenerator.connect( "VisibleNodes", preRenderFilter, "VisibleNodes" );
        preRenderFilter.getPromise( "Frustum" ).set( renderInputs.frameInfo.frustum );
        visibleSetGenerator.execute();
        preRenderFilter.execute();

        const livre::UniqueFutureMap portFutures( visibleSetGenerator.getPostconditions( ));
        NodeIds nodeIdsCopy = portFutures.get< NodeIds >( "VisibleNodes" );
        DistanceOperator distanceOp( renderInputs.dataSource, renderInputs.frameInfo.frustum );
        std::sort( nodeIdsCopy.begin(), nodeIdsCopy.end(), distanceOp );

        const VolumeInformation& volInfo = renderInputs.dataSource.getVolumeInfo();
        const size_t blockMemSize = volInfo.maximumBlockSize.product() *
                                    volInfo.getBytesPerVoxel() *
                                    volInfo.compCount;

        const uint32_t maxNodesPerPass =
                renderInputs.vrParameters.getMaxGPUCacheMemoryMB() * LB_1MB / blockMemSize;

        const uint32_t numberOfPasses = std::ceil( (float)nodeIdsCopy.size() / (float)maxNodesPerPass );

        std::unique_ptr< boost::progress_display > showProgress;
        if( numberOfPasses > 1 )
        {
            LBINFO << "Multipass rendering. Number of passes: " << numberOfPasses << std::endl;
            showProgress.reset( new boost::progress_display( numberOfPasses ));
        }

        sendHistogramFilter.getPromise( "RelativeViewport" ).set( renderInputs.viewport );
        sendHistogramFilter.getPromise( "Id" ).set( renderInputs.frameInfo.frameId );

        for( uint32_t i = 0; i < numberOfPasses; ++i )
        {
            uint32_t renderStages = RENDER_FRAME;
            if( i == 0 )
                renderStages |= RENDER_BEGIN;

            if( i == numberOfPasses - 1u )
                renderStages |= RENDER_END;

            const uint32_t startIndex = i * maxNodesPerPass;
            const uint32_t endIndex = ( i + 1 ) * maxNodesPerPass;
            const NodeIds nodesPerPass( nodeIdsCopy.begin() + startIndex,
                                        endIndex > nodeIdsCopy.size() ? nodeIdsCopy.end() :
                                        nodeIdsCopy.begin() + endIndex );

            createAndExecuteSyncPass( nodesPerPass,
                                      renderInputs,
                                      sendHistogramFilter,
                                      renderer,
                                      renderStages );
            if( numberOfPasses > 1 )
                ++(*showProgress);
        }
        sendHistogramFilter.schedule( _computeExecutor );

        const UniqueFutureMap futures( visibleSetGenerator.getPostconditions( ));
        statistics.nAvailable = futures.get< NodeIds >( "VisibleNodes" ).size();
        statistics.nNotAvailable = 0;
        statistics.nRenderAvailable = statistics.nAvailable;
    }

    void renderAsync( RenderStatistics& statistics,
                      Renderer& renderer,
                      const RenderInputs& renderInputs )
    {
        PipeFilter sendHistogramFilter = renderInputs.filters.find( "SendHistogramFilter" )->second;
        PipeFilter preRenderFilter = renderInputs.filters.find( "PreRenderFilter" )->second;
        PipeFilter redrawFilter = renderInputs.filters.find( "RedrawFilter" )->second;
        PipeFilterT< HistogramFilter > histogramFilter( "HistogramFilter",
                                                        renderInputs.histogramCache,
                                                        renderInputs.dataCache,
                                                        renderInputs.dataSource );
        histogramFilter.getPromise( "Frustum" ).set( renderInputs.frameInfo.frustum );
        histogramFilter.connect( "Histogram", sendHistogramFilter, "Histogram" );
        histogramFilter.getPromise( "RelativeViewport" ).set( renderInputs.viewport );
        histogramFilter.getPromise( "DataSourceRange" ).set( renderInputs.dataSourceRange );
        sendHistogramFilter.getPromise( "RelativeViewport" ).set( renderInputs.viewport );
        sendHistogramFilter.getPromise( "Id" ).set( renderInputs.frameInfo.frameId );
        preRenderFilter.getPromise( "Frustum" ).set( renderInputs.frameInfo.frustum );

        Pipeline renderPipeline;
        Pipeline uploadPipeline;

        PipeFilterT< RenderFilter > renderFilter( "RenderFilter",
                                                  renderInputs.dataSource,
                                                  renderer );

        PipeFilter visibleSetGenerator =
                renderPipeline.add< VisibleSetGeneratorFilter >(
                    "VisibleSetGenerator", renderInputs.dataSource );
        setupVisibleGeneratorFilter( visibleSetGenerator, renderInputs );

        PipeFilter renderingSetGenerator =
                renderPipeline.add< RenderingSetGeneratorFilter >(
                    "RenderingSetGenerator", *_textureCache );

        visibleSetGenerator.connect( "VisibleNodes", renderingSetGenerator, "VisibleNodes" );
        renderingSetGenerator.connect( "CacheObjects", renderFilter, "CacheObjects" );
        renderingSetGenerator.connect( "CacheObjects", histogramFilter, "CacheObjects" );
        renderingSetGenerator.connect( "RenderingDone", redrawFilter, "RenderingDone" );
        visibleSetGenerator.connect( "VisibleNodes", preRenderFilter, "VisibleNodes" );

        PipeFilter renderUploader = uploadPipeline.add< GLRenderUploadFilter >( "RenderUploader",
                                                                                *_textureCache,
                                                                                *_texturePool,
                                                                                nUploadThreads,
                                                                                _uploadExecutor );

        renderUploader.getPromise( "RenderInputs" ).set( renderInputs );
        visibleSetGenerator.connect( "VisibleNodes", renderUploader, "NodeIds" );

        renderFilter.getPromise( "RenderInputs" ).set( renderInputs );
        renderFilter.getPromise( "RenderStages" ).set( RENDER_ALL );

        redrawFilter.schedule( _renderExecutor );
        renderPipeline.schedule( _renderExecutor );
        uploadPipeline.schedule( _computeExecutor );
        sendHistogramFilter.schedule( _computeExecutor );
        histogramFilter.schedule( _computeExecutor );
        preRenderFilter.execute();
        renderFilter.execute();

        const UniqueFutureMap futures( renderingSetGenerator.getPostconditions( ));
        statistics = futures.get< RenderStatistics >( "RenderStatistics" );
    }

    void createAndExecuteSyncPass( NodeIds nodeIds,
                                   const RenderInputs& renderInputs,
                                   PipeFilter& sendHistogramFilter,
                                   Renderer& renderer,
                                   const uint32_t renderStages )
    {
        PipeFilterT< HistogramFilter > histogramFilter( "HistogramFilter",
                                                        renderInputs.histogramCache,
                                                        renderInputs.dataCache,
                                                        renderInputs.dataSource );
        histogramFilter.getPromise( "Frustum" ).set( renderInputs.frameInfo.frustum );
        histogramFilter.connect( "Histogram", sendHistogramFilter, "Histogram" );
        histogramFilter.getPromise( "RelativeViewport" ).set( renderInputs.viewport );
        histogramFilter.getPromise( "DataSourceRange" ).set( renderInputs.dataSourceRange );

        Pipeline renderPipeline;
        Pipeline uploadPipeline;

        PipeFilterT< RenderFilter > renderFilter( "RenderFilter",
                                                  renderInputs.dataSource,
                                                  renderer );
        renderFilter.getPromise( "RenderInputs" ).set( renderInputs );
        renderFilter.getPromise( "RenderStages" ).set( renderStages );

        PipeFilter renderUploader = uploadPipeline.add< GLRenderUploadFilter >( "RenderUploader",
                                                                                *_textureCache,
                                                                                *_texturePool,
                                                                                nUploadThreads,
                                                                                _uploadExecutor );

        renderUploader.getPromise( "RenderInputs" ).set( renderInputs );
        renderUploader.getPromise( "NodeIds" ).set( nodeIds );
        renderUploader.connect( "TextureCacheObjects", renderFilter, "CacheObjects" );
        renderUploader.connect( "TextureCacheObjects", histogramFilter, "CacheObjects" );

        renderPipeline.schedule( _renderExecutor );
        uploadPipeline.schedule( _uploadExecutor );
        histogramFilter.schedule( _computeExecutor );
        renderFilter.execute();
    }

    void initTextureCache( const RenderInputs& renderInputs )
    {
        if( _textureCache )
            return;

        const RendererParameters& vrParams = renderInputs.vrParameters;
        const size_t gpuMem = vrParams.getMaxGPUCacheMemoryMB() * LB_1MB;
        _textureCache.reset( new CacheT< TextureObject >( "TextureCache", gpuMem ));
        _texturePool.reset( new TexturePool( renderInputs.dataSource, gpuMem ));
    }

    void render( RenderStatistics& statistics,
                 Renderer& renderer,
                 const RenderInputs& renderInputs )
    {
        initTextureCache( renderInputs );
        if( renderInputs.vrParameters.getSynchronousMode( ))
            renderSync( statistics, renderer, renderInputs );
        else
            renderAsync( statistics, renderer, renderInputs );
    }

    SimpleExecutor _renderExecutor;
    SimpleExecutor _computeExecutor;
    SimpleExecutor _uploadExecutor;
    mutable std::unique_ptr< Cache > _textureCache;
    mutable std::unique_ptr< TexturePool > _texturePool;
};

GLRaycasterPipeline::GLRaycasterPipeline( const std::string& name )
    : RenderPipelinePlugin( name )
    , _impl( new GLRaycasterPipeline::Impl())
{}

GLRaycasterPipeline::~GLRaycasterPipeline()
{}

RenderStatistics GLRaycasterPipeline::render( Renderer& renderer, const RenderInputs& renderInputs )
{
    RenderStatistics statistics;
    _impl->render( statistics, renderer, renderInputs );
    return statistics;
}
}
