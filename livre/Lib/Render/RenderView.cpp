/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <livre/Lib/Render/RenderView.h>
#include <livre/Lib/Algorithm/LODFrustum.h>

#include <livre/Lib/Cache/TextureObject.h>
#include <livre/Lib/Visitor/LODSelectionVisitor.h>
#include <livre/Lib/Configuration/EFPrefetchAlgorithmParameters.h>
#include <livre/Lib/Algorithm/Optimizer.h>
#include <livre/Lib/types.h>

#include <livre/core/Render/Frustum.h>
#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Dash/DashTree.h>
#include <livre/core/Render/RenderingSetGenerator.h>
#include <livre/core/Maths/Maths.h>
#include <livre/core/Data/VolumeDataSource.h>
#include <livre/core/Data/VolumeInformation.h>
#include <livre/core/Render/GLWidget.h>

namespace livre
{

RenderView::RenderView( )
    : visibleFrustumScreenSpaceError_( 0 )
    , texturePFrustumScreenSpaceError_( 0 )
    , dataPFrustumScreenSpaceError_( 0 )
{
    const float frustumSurfaceDelta = 1.0f;
    prefetchDistanceRatios_.resize( PL_FAR + 1, frustumSurfaceDelta );
    prefetchDistanceRatios_[ PL_NEAR ] = 0.1f;
}

void RenderView::setParameters( ConstVolumeRendererParametersPtr volumeRendererParameters,
                                ConstEFPParametersPtr prefetchAlgorithmParameters )
{
    volumeRendererParameters_ = *volumeRendererParameters;
    prefetchAlgorithmParameters_ = *prefetchAlgorithmParameters;

    const float screenSpaceError = volumeRendererParameters_.screenSpaceError;
    texturePFrustumScreenSpaceError_ = screenSpaceError;
    dataPFrustumScreenSpaceError_ = screenSpaceError;
    visibleFrustumScreenSpaceError_ = screenSpaceError;

}

void RenderView::onPostRender_( const bool /*rendered*/,
                                const GLWidget& widget,
                                const FrameInfo& frameInfo,
                                RenderingSetGenerator& renderSetGenerator )
{
    if( frameInfo.previousFrustum != frameInfo.currentFrustum )
         freeTextures_( frameInfo.renderNodeList );

    Viewporti pixelViewport;
    widget.setViewport( this, pixelViewport );

    const uint32_t windowHeight = pixelViewport.getHeight();
    generateRequest_( frameInfo.currentFrustum, renderSetGenerator, windowHeight );
}

void RenderView::freeTextures_( const DashNodeVector& renderNodeList )
{
    DashNodeSet currentSet;
    for( DashNodeVector::const_iterator it = renderNodeList.begin();
         it != renderNodeList.end(); ++it )
    {
        dash::NodePtr node = *it;
        currentSet.insert( node );
    }

    DashNodeSet cleanSet;
    std::set_difference( previousVisibleSet_.begin(),
                         previousVisibleSet_.end(),
                         currentSet.begin(),
                         currentSet.end(),
                         std::inserter( cleanSet, cleanSet.begin( )));

    // Unreference not needed textures
    for( DashNodeSet::iterator it = cleanSet.begin(); it != cleanSet.end(); ++it )
    {
        DashRenderNode renderNode( *it );
        if( renderNode.getLODNode().getRefLevel() != 0 )
            renderNode.setTextureObject( TextureObject::getEmptyPtr( ));
    }

    previousVisibleSet_ = currentSet;
}

void RenderView::generateRequest_( const Frustum& currentFrustum,
                                   RenderingSetGenerator& renderSetGenerator,
                                   const uint32_t windowHeight )
{

    FloatVector distances;
    const float frustumSurfaceDelta = 0.0f;
    distances.resize( PL_FAR + 1, frustumSurfaceDelta );

    const VolumeInformation& volumeInfo =
            renderSetGenerator.getDashTree()->getDataSource()->getVolumeInformation();

    const float wsPerVoxel = volumeInfo.worldSpacePerVoxel;
    const float depth = volumeInfo.rootNode.getDepth();
    const float levelZeroNodeSize = float( volumeInfo.maximumBlockSize[ 0 ] ) *
                                    volumeInfo.worldSpacePerVoxel;

    const LODFrustum renderFrustum( currentFrustum,
                                    visibleFrustumScreenSpaceError_,
                                    windowHeight,
                                    wsPerVoxel,
                                    levelZeroNodeSize,
                                    depth,
                                    distances );
    DashTreePtr dashTree = renderSetGenerator.getDashTree();
    LODSelectionVisitor renderVisitor( dashTree, renderFrustum );

    dfsTraverser_.traverse( volumeInfo.rootNode, renderVisitor );
}

}
