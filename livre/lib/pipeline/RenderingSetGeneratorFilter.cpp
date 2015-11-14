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

#include <livre/lib/pipeline/RenderingSetGeneratorFilter.h>
#include <livre/lib/pipeline/RenderPipeFilter.h>
#include <livre/lib/render/AvailableSetGenerator.h>
#include <livre/lib/cache/TextureCache.h>

#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/FilterInput.h>
#include <livre/core/pipeline/FilterOutput.h>
#include <livre/core/pipeline/InputPort.h>
#include <livre/core/pipeline/Workers.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/render/VisibleSetGenerator.h>

namespace livre
{

struct RenderingSetGeneratorFilter::Impl
{
    Impl() {}

    void execute( PipeFilter& pipeFilter ) const
    {
        const RenderPipeFilter::RenderPipeInput& renderPipeInput =
                pipeFilter.getInputValue< RenderPipeFilter::RenderPipeInput >( "RenderPipeInput" );
        pipeFilter.setOutput( "RenderPipeInput", renderPipeInput );

        const NodeIds& visibleNodes = pipeFilter.getInputValue< NodeIds >( "VisibleNodes" );
        const Frustum& frustum = pipeFilter.getInputValue< Frustum >( "Frustum" );
        const uint32_t frame = pipeFilter.getInputValue< uint32_t >( "Frame" );

        FrameInfo frameInfo( frustum, frame );
        AvailableSetGenerator renderSetGenerator(
                    static_cast<TextureCache&>(*renderPipeInput.textureCache ));
        renderSetGenerator.generateRenderingSet( frameInfo,
                                                 visibleNodes );

        pipeFilter.setOutput( "CacheObjects", std::move( frameInfo.renderNodes ));

    }

    void getInputPorts( PortInfos& inputPorts ) const
    {
        addPortInfo( inputPorts, "RenderPipeInput", RenderPipeFilter::RenderPipeInput( ));
        addPortInfo( inputPorts, "Frustum", Frustum( ));
        addPortInfo( inputPorts, "Frame", uint32_t( INVALID_FRAME ));
        addPortInfo( inputPorts, "VisibleNodes", NodeIds());
    }

    void getOutputPorts( PortInfos& outputPorts ) const
    {
        addPortInfo( outputPorts, "RenderPipeInput", RenderPipeFilter::RenderPipeInput( ));
        addPortInfo( outputPorts, "CacheObjects", ConstCacheObjects());
    }
};

RenderingSetGeneratorFilter::RenderingSetGeneratorFilter()
    : _impl( new RenderingSetGeneratorFilter::Impl( ))
{
}

RenderingSetGeneratorFilter::~RenderingSetGeneratorFilter()
{

}
void RenderingSetGeneratorFilter::getInputPorts( PortInfos& inputPorts ) const
{
    _impl->getInputPorts( inputPorts );
}

void RenderingSetGeneratorFilter::getOutputPorts( PortInfos& outputPorts ) const
{
    _impl->getOutputPorts( outputPorts );
}

void RenderingSetGeneratorFilter::execute( PipeFilter& filter ) const
{
    _impl->execute( filter );
}


}
