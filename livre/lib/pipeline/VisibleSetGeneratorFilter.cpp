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

#include <livre/lib/pipeline/VisibleSetGeneratorFilter.h>
#include <livre/lib/pipeline/RenderPipeFilter.h>
#include <livre/lib/render/ScreenSpaceLODEvaluator.h>
#include <livre/lib/cache/TextureCache.h>

#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/FilterInput.h>
#include <livre/core/pipeline/FilterOutput.h>
#include <livre/core/pipeline/InputPort.h>
#include <livre/core/pipeline/Workers.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/render/VisibleSetGenerator.h>
#include <livre/core/render/GLWidget.h>

namespace livre
{

struct VisibleSetGeneratorFilter::Impl
{
    Impl() {}

    void execute( PipeFilter& pipeFilter ) const
    {
        const RenderPipeFilter::RenderPipeInput& renderPipeInput =
                pipeFilter.getInputValue< RenderPipeFilter::RenderPipeInput >( "RenderPipeInput" );
        pipeFilter.setOutput( "RenderPipeInput", renderPipeInput );

        const NodeId& rootNodeId = pipeFilter.getInputValue< NodeId >( "NodeId" );
        const Frustum& frustum = pipeFilter.getInputValue< Frustum >( "Frustum" );
        const uint32_t frame = pipeFilter.getInputValue< uint32_t >( "Frame" );
        const Vector2f& dataRange = pipeFilter.getInputValue< Vector2f >( "DataRange" );

        const PixelViewport viewport( renderPipeInput.glWidget->getX(),
                                      renderPipeInput.glWidget->getY(),
                                      renderPipeInput.glWidget->getWidth(),
                                      renderPipeInput.glWidget->getHeight());

        ScreenSpaceLODEvaluator lodEvaluator( renderPipeInput.screenSpaceError );

        NodeIds visibleNodes;
        VisibleSetGenerator visibleSetGenerator;
        visibleSetGenerator.generateVisibleSet( viewport,
                                                frustum,
                                                lodEvaluator,
                                                *renderPipeInput.dataSource,
                                                frame,
                                                renderPipeInput.minLOD,
                                                renderPipeInput.maxLOD,
                                                visibleNodes,
                                                dataRange,
                                                rootNodeId );


        pipeFilter.setOutput( "VisibleNodes", std::move( visibleNodes ));
    }

    void getInputPorts( PortInfos& inputPorts ) const
    {
        addPortInfo( inputPorts, "RenderPipeInput", RenderPipeFilter::RenderPipeInput( ));
        addPortInfo( inputPorts, "Frustum", Frustum( ));
        addPortInfo( inputPorts, "Frame", uint32_t( INVALID_FRAME ));
        addPortInfo( inputPorts, "NodeId", NodeId());
        addPortInfo( inputPorts, "DataRange", Vector2f( 0.f, 1.f ));
    }

    void getOutputPorts( PortInfos& outputPorts ) const
    {
        addPortInfo( outputPorts, "RenderPipeInput", RenderPipeFilter::RenderPipeInput( ));
        addPortInfo( outputPorts, "VisibleNodes", NodeIds());
    }
};

VisibleSetGeneratorFilter::VisibleSetGeneratorFilter()
    : _impl( new VisibleSetGeneratorFilter::Impl( ))
{
}

VisibleSetGeneratorFilter::~VisibleSetGeneratorFilter()
{

}
void VisibleSetGeneratorFilter::getInputPorts( PortInfos& inputPorts ) const
{
    _impl->getInputPorts( inputPorts );
}

void VisibleSetGeneratorFilter::getOutputPorts( PortInfos& outputPorts ) const
{
    _impl->getOutputPorts( outputPorts );
}

void VisibleSetGeneratorFilter::execute( PipeFilter& filter ) const
{
    _impl->execute( filter );
}


}
