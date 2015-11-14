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

#include <livre/lib/pipeline/DataUploadFilter.h>
#include <livre/lib/pipeline/RenderPipeFilter.h>

#include <livre/lib/cache/TextureDataCache.h>
#include <livre/lib/cache/TextureCache.h>
#include <livre/lib/cache/TextureDataObject.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/pipeline/Pipeline.h>
#include <livre/core/pipeline/FilterInput.h>
#include <livre/core/pipeline/FilterOutput.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/InputPort.h>
#include <livre/core/pipeline/Executor.h>

#include <livre/core/data/NodeId.h>
#include <livre/core/data/VolumeDataSource.h>

namespace livre
{

class TextureDataUploadFilter : public Filter
{
public:

    TextureDataUploadFilter( CachePtr dataCache );
    void execute( PipeFilter& filter ) const final;

    void getInputPorts( PortInfos& inputPorts ) const final;
    void getOutputPorts( PortInfos& outputPorts ) const final;

private:
    mutable CachePtr _dataCache;
};

class TextureUploadFilter : public Filter
{
public:

    TextureUploadFilter( CachePtr textureCache );
    void execute( PipeFilter& filter ) const final;

    void getInputPorts( PortInfos& inputPorts ) const final;
    void getOutputPorts( PortInfos& outputPorts ) const final;

private:
    mutable CachePtr _textureCache;
};

TextureDataUploadFilter::TextureDataUploadFilter( CachePtr dataCache )
    : _dataCache( dataCache )
{}

void TextureDataUploadFilter::execute( PipeFilter& filter ) const
{
    const CacheId& cacheId = filter.getInputValue<CacheId>( "CacheId" );

    CacheObjectPtr cacheObject =
            boost::static_pointer_cast< TextureDataObject >(
                _dataCache->getObjectFromCache( cacheId ));

    if( !cacheObject->isLoaded( ))
        cacheObject->cacheLoad( );

    filter.setOutput( "TextureDataObject",
                      boost::static_pointer_cast<TextureDataObject>(cacheObject));
}
void TextureDataUploadFilter::getInputPorts( PortInfos& inputPorts ) const
{
    addPortInfo( inputPorts, "CacheId", CacheId( INVALID_CACHE_ID ));
}

void TextureDataUploadFilter::getOutputPorts( PortInfos& outputPorts ) const
{
    addPortInfo( outputPorts, "TextureDataObject", TextureDataObjectPtr());
}

TextureUploadFilter::TextureUploadFilter( CachePtr textureCache )
    : _textureCache( textureCache )
{}

void TextureUploadFilter::execute( PipeFilter& filter ) const
{

    const TextureDataObjectPtr& textureData =
            filter.getInputValue<TextureDataObjectPtr>( "TextureDataObject" );

    CacheObjectPtr cacheObject =
           boost::static_pointer_cast< TextureObject >(
                _textureCache->getObjectFromCache( textureData->getCacheId( )));

    if( !cacheObject->isLoaded( ))
    {
        boost::static_pointer_cast< TextureObject >( cacheObject )
                ->setTextureDataObject(  boost::static_pointer_cast<
                                         const TextureDataObject >( textureData ));

        cacheObject->cacheLoad();
    }

    filter.setOutput( "TextureObject",
                      boost::static_pointer_cast<TextureObject>(cacheObject) );
}

void TextureUploadFilter::getInputPorts( PortInfos& inputPorts ) const
{
    addPortInfo( inputPorts, "TextureDataObject", TextureDataObjectPtr( ));
}

void TextureUploadFilter::getOutputPorts( PortInfos& outputPorts ) const
{
    addPortInfo( outputPorts, "TextureObject", TextureObjectPtr( ));
}

struct DataUploadFilter::Impl
{
   void execute( PipeFilter& pipeFilter ) const
    {
        const RenderPipeFilter::RenderPipeInput& renderPipeInput =
                pipeFilter.getInputValue< RenderPipeFilter::RenderPipeInput >( "RenderPipeInput" );
        pipeFilter.setOutput( "RenderPipeInput", renderPipeInput );

        const NodeIds& visibleNodeIds = pipeFilter.getInputValue< NodeIds >( "VisibleNodes" );

        FilterPtr dataUploadFilter( new TextureDataUploadFilter( renderPipeInput.dataCache ));
        FilterPtr textureUploadFilter( new TextureUploadFilter( renderPipeInput.textureCache ));

        ConstCacheObjects allCacheObjects;

        renderPipeInput.uploadExecutor->clear();

        PipeFilterPtrs filters;
        BOOST_FOREACH( const NodeId& nodeId, visibleNodeIds )
        {
            PipelinePtr uploadDataPipeline( new Pipeline( ));
            const TextureObjectPtr& textureCacheObject =
                    boost::static_pointer_cast<TextureObject>(
                        renderPipeInput.textureCache->getObjectFromCache( nodeId.getId( )));

            if( textureCacheObject->isLoaded( ))
            {
                allCacheObjects.push_back( textureCacheObject );
                continue;
            }

            const CacheId& cacheId = nodeId.getId();

            const TextureDataObjectPtr& textureData =
                    boost::static_pointer_cast<TextureDataObject>(
                        renderPipeInput.dataCache->getObjectFromCache( cacheId ));

            if( !textureData->isLoaded( ))
            {
                auto dataUploader = uploadDataPipeline->add( dataUploadFilter, false );
                auto textureUploader = uploadDataPipeline->add( textureUploadFilter, false );
                connectFilters( dataUploader, textureUploader, "TextureDataObject" );
                dataUploader->setInput( "CacheId", cacheId );
                filters.push_back( textureUploader );
            }
            else
            {
                auto textureUploader = uploadDataPipeline->add( textureUploadFilter, false );
                textureUploader->setInput( "TextureDataObject", textureData );
                filters.push_back( textureUploader );
            }

            PipelinePtr uploadDataSynchronousPipeline( new Pipeline( ));
            uploadDataSynchronousPipeline->add( uploadDataPipeline, false );
            renderPipeInput.uploadExecutor->submit( uploadDataSynchronousPipeline );
        }

        if( renderPipeInput.isSynchronous )
        {
            BOOST_FOREACH( const PipeFilterPtr& filter, filters )
                allCacheObjects.push_back( filter->getOutputValue<TextureObjectPtr>( "TextureObject" ));
        }

        pipeFilter.setOutput( "CacheObjects", std::move( allCacheObjects ));
    }

    void getInputPorts( PortInfos& inputPorts ) const
    {
        addPortInfo( inputPorts, "RenderPipeInput", RenderPipeFilter::RenderPipeInput( ));
        addPortInfo( inputPorts, "VisibleNodes", NodeIds( ));
    }

    void getOutputPorts( PortInfos& outputPorts ) const
    {
        addPortInfo( outputPorts, "RenderPipeInput", RenderPipeFilter::RenderPipeInput( ));
        addPortInfo( outputPorts, "CacheObjects", ConstCacheObjects( ));
    }
};

DataUploadFilter::DataUploadFilter()
    : _impl( new DataUploadFilter::Impl())
{}

DataUploadFilter::~DataUploadFilter()
{}


void DataUploadFilter::execute( PipeFilter& filter ) const
{
    _impl->execute( filter );
}

void DataUploadFilter::getInputPorts( PortInfos& inputPorts ) const
{
    _impl->getInputPorts( inputPorts );
}

void DataUploadFilter::getOutputPorts( PortInfos& outputPorts ) const
{
    _impl->getOutputPorts( outputPorts );
}

}
