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

#include <livre/lib/pipeline/RenderPipeline.h>
#include <livre/lib/pipeline/RenderingSetGeneratorFilter.h>
#include <livre/lib/pipeline/VisibleSetGeneratorFilter.h>
#include <livre/lib/pipeline/DataUploadFilter.h>
#include <livre/lib/pipeline/RenderFilter.h>

#include <livre/lib/cache/TextureCache.h>
#include <livre/lib/cache/TextureDataCache.h>

#include <livre/core/pipeline/SimpleExecutor.h>
#include <livre/core/pipeline/Pipeline.h>

namespace livre
{

struct RenderPipeline::Impl
{
    Impl( TextureCache& textureCache,
          const size_t nComputeThreads,
          const size_t nUploadThreads,
          ConstGLContextPtr glContext )
        : _textureCache( textureCache )
        , _dataSource( textureCache.getDataCache().getDataSource( ))
        , _computeExecutor( nComputeThreads, glContext )
        , _uploadExecutor( nUploadThreads, glContext )
        , _nUploadThreads( nUploadThreads )
    {
    }

    void createAndConnectUploaders( Pipeline& uploadPipeline,
                                    PipeFilter& visibleSetGenerator,
                                    PipeFilter& output  ) const
    {
        for( size_t i = 0; i < _nUploadThreads; ++i )
        {
            std::stringstream name;
            name << "DataUploader" << i;
            PipeFilter uploader =
                    uploadPipeline.add< DataUploadFilter >( name.str(),
                                                            i,
                                                            _nUploadThreads,
                                                            _textureCache );
            visibleSetGenerator.connect( "VisibleNodes",
                                         uploader, "VisibleNodes" );
            visibleSetGenerator.connect( "Params",
                                         uploader, "Params" );
            uploader.connect( "CacheObjects", output, "CacheObjects" );
        }
    }

    void createSyncPipeline( PipeFilter& renderFilter,
                             Pipeline& computePipeline,
                             Pipeline& uploadPipeline ) const
    {
        PipeFilter visibleSetGenerator =
                computePipeline.add< VisibleSetGeneratorFilter >(
                    "VisibleSetGenerator", _dataSource );

        createAndConnectUploaders( uploadPipeline,
                                   visibleSetGenerator,
                                   renderFilter );
    }

    void createAsyncPipeline( PipeFilter& renderFilter,
                              PipeFilter& redrawFilter,
                              Pipeline& computePipeline,
                              Pipeline& uploadPipeline ) const
    {
        PipeFilter visibleSetGenerator =
                computePipeline.add< VisibleSetGeneratorFilter >(
                    "VisibleSetGenerator", _dataSource );

        PipeFilter renderingSetGenerator =
                computePipeline.add< RenderingSetGeneratorFilter >(
                    "RenderingSetGenerator", _textureCache );

        visibleSetGenerator.connect( "VisibleNodes",
                                     renderingSetGenerator, "VisibleNodes" );

        renderingSetGenerator.connect( "CacheObjects",
                                      renderFilter, "CacheObjects" );

        renderingSetGenerator.connect( "RenderingDone",
                                       redrawFilter, "RenderingDone" );

        createAndConnectUploaders( uploadPipeline,
                                   visibleSetGenerator,
                                   redrawFilter );
    }

    void render( const VolumeRendererParameters& vrParams,
                 const FrameInfo& frameInfo,
                 const Range& dataRange,
                 const PixelViewport& pixelViewPort,
                 PipeFilter redrawFilter,
                 Renderer& renderer,
                 size_t& nAvailable,
                 size_t& nNotAvailable ) const
    {

        PipeFilterT< RenderFilter > renderFilter( "RenderFilter", _dataSource, renderer );

        Pipeline computePipeline;
        Pipeline uploadPipeline;

        if( vrParams.getSynchronousMode( ))
            createSyncPipeline( renderFilter,
                                computePipeline,
                                uploadPipeline );
        else
            createAsyncPipeline( renderFilter,
                                 redrawFilter,
                                 computePipeline,
                                 uploadPipeline );

        PipeFilter visibleSetGenerator =
                static_cast< const livre::PipeFilter& >(
                    computePipeline.getExecutable( "VisibleSetGenerator" ));

        visibleSetGenerator.getPromise( "Frustum" ).set( frameInfo.frustum );
        visibleSetGenerator.getPromise( "Frame" ).set( frameInfo.frameId );
        visibleSetGenerator.getPromise( "DataRange" ).set( dataRange );
        visibleSetGenerator.getPromise( "Params" ).set( vrParams );
        visibleSetGenerator.getPromise( "Viewport" ).set( pixelViewPort );

        renderFilter.getPromise( "Frustum" ).set( frameInfo.frustum );
        renderFilter.getPromise( "Viewport" ).set( pixelViewPort );

        if( !vrParams.getSynchronousMode( ))
            redrawFilter.schedule( _computeExecutor );
        computePipeline.schedule( _computeExecutor );
        uploadPipeline.schedule( _uploadExecutor );

        renderFilter.execute();

        if( vrParams.getSynchronousMode( ))
        {
            const UniqueFutureMap futures( visibleSetGenerator.getPostconditions( ));
            nAvailable = futures.get< NodeIds >( "VisibleNodes" ).size();
            nNotAvailable = 0;
        }
        else
        {
            const PipeFilter renderingSetGenerator =
                    static_cast< const livre::PipeFilter& >(
                        computePipeline.getExecutable( "RenderingSetGenerator" ));

            const UniqueFutureMap futures( renderingSetGenerator.getPostconditions( ));
            nAvailable = futures.get< size_t >( "AvailableCount" );
            nNotAvailable = futures.get< size_t >( "NotAvailableCount" );
        }
    }

    TextureCache& _textureCache;
    const DataSource& _dataSource;
    mutable SimpleExecutor _computeExecutor;
    mutable SimpleExecutor _uploadExecutor;
    const size_t _nUploadThreads;
};

RenderPipeline::RenderPipeline( TextureCache& textureCache,
                                const size_t nComputeThreads,
                                const size_t nUploadThreads,
                                ConstGLContextPtr glContext )
    : _impl( new RenderPipeline::Impl( textureCache,
                                       nComputeThreads,
                                       nUploadThreads,
                                       glContext ))
{}

RenderPipeline::~RenderPipeline()
{}

void RenderPipeline::render( const VolumeRendererParameters& vrParams,
                             const FrameInfo& frameInfo,
                             const Range& dataRange,
                             const PixelViewport& pixelViewPort,
                             const PipeFilter& redrawFilter,
                             Renderer& renderer,
                             size_t& nAvailable,
                             size_t& nNotAvailable ) const
{
    _impl->render( vrParams,
                   frameInfo,
                   dataRange,
                   pixelViewPort,
                   redrawFilter,
                   renderer,
                   nAvailable,
                   nNotAvailable );
}

}
