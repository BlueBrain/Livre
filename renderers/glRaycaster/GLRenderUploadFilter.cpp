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

#include "GLRenderUploadFilter.h"
#include <livre/lib/pipeline/DataUploadFilter.h>
#include <livre/lib/pipeline/TextureUploadFilter.h>

#include <livre/lib/cache/DataObject.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/configuration/RendererParameters.h>
#include <livre/core/pipeline/Pipeline.h>
#include <livre/core/pipeline/SimpleExecutor.h>
#include <livre/core/data/NodeId.h>
#include <livre/core/cache/Cache.h>
#include <livre/core/render/GLContext.h>

#include <GL/glew.h>

namespace livre
{

struct GLRenderUploadFilter::Impl
{
public:

    Impl( Cache& textureCache,
          TexturePool& texturePool,
          size_t nUploadThreads,
          Executor& executor )
        : _textureCache( textureCache )
        , _texturePool( texturePool )
        , _nUploadThreads( nUploadThreads )
        , _executor( executor )
    {}

    void execute( const FutureMap& input, PromiseMap& output ) const
    {
        const UniqueFutureMap futureMap( input.getFutures( ));
        ConstCacheObjects cacheObjects; // Lock
        NodeIds notAvailable;
        const auto& renderInputs = futureMap.get< RenderInputs >( "RenderInputs" );
        for( const auto& nodeId: futureMap.get< NodeIds >( "NodeIds" ))
        {
            const auto& cacheObj = _textureCache.load< TextureObject >( nodeId.getId(),
                                                                        renderInputs.dataCache,
                                                                        renderInputs.dataSource,
                                                                        _texturePool );
            if( cacheObj )
                cacheObjects.push_back( cacheObj );
            else
                notAvailable.push_back( nodeId );
        }

        if( notAvailable.empty( ))
        {
             output.set( "TextureCacheObjects", cacheObjects );
             return;
        }

        const size_t perThreadSize = std::max( (size_t)1, notAvailable.size() / _nUploadThreads );
        Pipeline pipeline;
        PipeFilter textureUploader = pipeline.add< TextureUploadFilter >( "TextureUploader",
                                                                          _textureCache,
                                                                          _texturePool );
        textureUploader.getPromise( "RenderInputs" ).set( renderInputs );
        for( size_t i = 0; i < _nUploadThreads; ++i )
        {
            if( i * perThreadSize >= notAvailable.size( ))
                continue;

            const NodeId* begin = notAvailable.data() + perThreadSize * i;
            NodeIds partialData;
            if( i == _nUploadThreads - 1 ) // last item
               partialData = NodeIds( begin,
                                      begin + (notAvailable.size() - ( perThreadSize * i )));
            else
               partialData = NodeIds( begin,
                                      begin + perThreadSize );
            std::stringstream str;
            str << "DataUploader" << i;
            PipeFilter dataUploader = pipeline.add< DataUploadFilter >( str.str( ));
            dataUploader.connect( "DataCacheObjects", textureUploader, "DataCacheObjects" );
            dataUploader.getPromise( "RenderInputs" ).set( renderInputs );
            dataUploader.getPromise( "NodeIds" ).set( partialData );
        }

        pipeline.schedule( _executor );
        UniqueFutureMap uploaderFutureMap( textureUploader.getPostconditions( ));
        const auto& textureCacheObjects =
                uploaderFutureMap.get< ConstCacheObjects >( "TextureCacheObjects" );
        cacheObjects.insert( cacheObjects.end(),
                             textureCacheObjects.begin(),
                             textureCacheObjects.end( ));

        output.set( "TextureCacheObjects", cacheObjects );
    }

    Cache& _textureCache;
    TexturePool& _texturePool;
    const size_t _nUploadThreads;
    Executor& _executor;
};

GLRenderUploadFilter::GLRenderUploadFilter( Cache& textureCache,
                                            TexturePool& texturePool,
                                            size_t nUploadThreads,
                                            Executor& executor )
    : _impl( new GLRenderUploadFilter::Impl( textureCache,
                                             texturePool,
                                             nUploadThreads,
                                             executor ))
{
}

GLRenderUploadFilter::~GLRenderUploadFilter()
{}

void GLRenderUploadFilter::execute( const FutureMap& input, PromiseMap& output ) const
{
    _impl->execute( input, output );
}
}

