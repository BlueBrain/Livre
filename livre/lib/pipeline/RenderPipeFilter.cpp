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

#include <livre/lib/pipeline/RenderPipeFilter.h>
#include <livre/lib/pipeline/RenderingSetGeneratorFilter.h>
#include <livre/lib/pipeline/RenderFilter.h>
#include <livre/lib/pipeline/VisibleSetGeneratorFilter.h>
#include <livre/lib/pipeline/DataUploadFilter.h>
#include <livre/lib/cache/TextureCache.h>
#include <livre/lib/cache/TextureDataCache.h>
#include <livre/lib/cache/TextureDataObject.h>
#include <livre/lib/cache/TextureObject.h>
#include <livre/core/pipeline/Pipeline.h>
#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/Executor.h>
#include <livre/core/cache/CacheObject.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/data/VolumeInformation.h>

namespace livre
{

struct RenderPipeFilter::Impl
{
    Impl()
    {}

    void execute( PipeFilter& pipeFilter ) const
    {
        const RenderPipeFilter::RenderPipeInput& renderPipeInput =
                pipeFilter.getInputValue< RenderPipeFilter::RenderPipeInput >( "RenderPipeInput" );
        pipeFilter.setOutput( "RenderPipeInput", renderPipeInput );
        const Frustum& frustum = pipeFilter.getInputValue< Frustum >( "Frustum" );
        const uint32_t frame = pipeFilter.getInputValue< uint32_t >( "Frame" );
        const Vector2f& dataRange = pipeFilter.getInputValue< Vector2f >( "DataRange" );

        FilterPtr visibleSetGenerator( new VisibleSetGeneratorFilter( ));
        FilterPtr renderingSetGenerator( new RenderingSetGeneratorFilter( ));
        FilterPtr uploadPipeFilter( new DataUploadFilter( ));
        FilterPtr renderFilter( new RenderFilter( ));

        PipelinePtr pipeline( new Pipeline( ));
        PipeFilterPtr renderPipeFilter( new PipeFilter( renderFilter ));
        renderPipeFilter->setInput( "RenderPipeInput", renderPipeInput );

        const VolumeInformation& volInfo = renderPipeInput.dataSource->getVolumeInformation();
        const Vector3ui& blockSize = volInfo.rootNode.getBlockSize();
        for( uint32_t x = 0; x < blockSize[0]; ++x )
            for( uint32_t y = 0; y < blockSize[1]; ++y )
                for( uint32_t z = 0; z < blockSize[2]; ++z )
                {
                    const NodeId rootNode( 0,
                                           Vector3ui( x, y, z ),
                                           frame );
                    PipeFilterPtr visPipeFilter = pipeline->add( visibleSetGenerator );
                    visPipeFilter->setInput( "RenderPipeInput", renderPipeInput );
                    visPipeFilter->setInput( "Frustum", frustum );
                    visPipeFilter->setInput( "Frame", frame );
                    visPipeFilter->setInput( "NodeId", rootNode );
                    visPipeFilter->setInput( "DataRange", dataRange );

                    // Pipeline does not wait for uploader unless explicitly waited by another filter
                    PipeFilterPtr uploadDataFilter = pipeline->add( uploadPipeFilter, false );
                    uploadDataFilter->setInput( "RenderPipeInput", renderPipeInput );
                    livre::connectFilters( visPipeFilter, uploadDataFilter, "VisibleNodes" );
                    if( !renderPipeInput.isSynchronous )
                    {
                        PipeFilterPtr renderSetGenPipeFilter = pipeline->add( renderingSetGenerator );
                        renderSetGenPipeFilter->setInput( "RenderPipeInput", renderPipeInput );
                        renderSetGenPipeFilter->setInput( "Frustum", frustum );
                        renderSetGenPipeFilter->setInput( "Frame", frame );
                        livre::connectFilters( visPipeFilter, renderSetGenPipeFilter, "VisibleNodes" );
                        livre::connectFilters( renderSetGenPipeFilter, renderPipeFilter, "CacheObjects" );
                    }
                    else
                        livre::connectFilters( uploadDataFilter, renderPipeFilter, "CacheObjects" );

                }

        if( !renderPipeInput.isSynchronous )
        {
            PipeFilterPtr redrawPipeFilter = pipeline->add( renderPipeInput.redrawFilter );
            redrawPipeFilter->setInput( "RenderPipeInput", renderPipeInput );
            livre::connectFilters( renderPipeFilter, redrawPipeFilter );
        }

        renderPipeInput.computeExecutor->execute( pipeline );
        renderPipeFilter->execute();
    }

    void getInputPorts( PortInfos& inputPorts ) const
    {
        addPortInfo( inputPorts, "RenderPipeInput", RenderPipeFilter::RenderPipeInput( ));
        addPortInfo( inputPorts, "Frustum", Frustum( ));
        addPortInfo( inputPorts, "Frame", uint32_t( INVALID_FRAME ));
        addPortInfo( inputPorts, "DataRange", Vector2f( 0.f, 1.f ));
    }

    void getOutputPorts( PortInfos& outputPorts ) const
    {
        addPortInfo( outputPorts, "RenderPipeInput", RenderPipeFilter::RenderPipeInput( ));
        addPortInfo( outputPorts, "DataRange", Frustum( ));
    }
};

RenderPipeFilter::RenderPipeFilter( )
    : _impl( new RenderPipeFilter::Impl( ))
{
}

void RenderPipeFilter::getInputPorts( PortInfos& inputPorts ) const
{
    _impl->getInputPorts( inputPorts );
}

void RenderPipeFilter::getOutputPorts( PortInfos& outputPorts ) const
{
    _impl->getOutputPorts( outputPorts );
}

RenderPipeFilter::~RenderPipeFilter()
{

}

void RenderPipeFilter::execute( PipeFilter& pipeFilter ) const
{
    _impl->execute( pipeFilter );
}


}
