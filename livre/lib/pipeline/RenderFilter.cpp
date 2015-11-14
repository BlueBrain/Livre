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

#include <livre/lib/pipeline/RenderFilter.h>
#include <livre/lib/pipeline/RenderPipeFilter.h>
#include <livre/lib/cache/TextureObject.h>
#include <livre/core/render/RenderBrick.h>
#include <livre/core/render/View.h>
#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/data/LODNode.h>

namespace livre
{

struct RenderFilter::Impl
{
    Impl()
    {}

    void execute( PipeFilter& pipeFilter ) const
    {
        const RenderPipeFilter::RenderPipeInput& renderPipeInput =
                pipeFilter.getInputValue< RenderPipeFilter::RenderPipeInput >( "RenderPipeInput" );
        pipeFilter.setOutput( "RenderPipeInput", renderPipeInput );

        const size_t inputSize = pipeFilter.getInputSize( "CacheObjects" );

        RenderBricks renderBricks;
        for( size_t i = 0; i < inputSize; ++i )
        {
            const ConstCacheObjects& cacheObjects =
                    pipeFilter.getInputValue<ConstCacheObjects>( "CacheObjects", i );

            BOOST_FOREACH( const ConstCacheObjectPtr& cacheObject, cacheObjects )
            {
                const ConstTextureObjectPtr& texture =
                               boost::static_pointer_cast< const TextureObject >( cacheObject );

                const LODNode& lodNode =
                        renderPipeInput.dataSource->getNode( NodeId( texture->getCacheId( )));

                RenderBrickPtr renderBrick(
                            new RenderBrick( lodNode,
                                             texture->getTextureState( )));
                renderBricks.push_back( renderBrick );
            }
        }

        renderPipeInput.view->render( renderBricks, *renderPipeInput.glWidget );
    }

    void getInputPorts( PortInfos& inputPorts ) const
    {
        addPortInfo( inputPorts, "RenderPipeInput", RenderPipeFilter::RenderPipeInput( ));
        addPortInfo( inputPorts,  "CacheObjects", ConstCacheObjects( ));
    }


    void getOutputPorts( PortInfos& outputPorts ) const
    {
        addPortInfo( outputPorts, "RenderPipeInput", RenderPipeFilter::RenderPipeInput( ));
    }
};

RenderFilter::RenderFilter()
    : _impl( new RenderFilter::Impl())
{
}

void RenderFilter::getInputPorts( PortInfos& inputPorts ) const
{
    _impl->getInputPorts( inputPorts );
}

void RenderFilter::getOutputPorts( PortInfos& outputPorts ) const
{
    _impl->getOutputPorts( outputPorts );
}

RenderFilter::~RenderFilter()
{

}

void RenderFilter::execute( PipeFilter& pipeFilter ) const
{
    _impl->execute( pipeFilter );
}


}
