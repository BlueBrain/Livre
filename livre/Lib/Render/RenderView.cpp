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
#include <livre/core/Render/Frustum.h>
#include <livre/Lib/Algorithm/LODFrustum.h>
#include <livre/core/Dash/DashRenderNode.h>
#include <livre/Lib/Cache/TextureObject.h>
#include <livre/Lib/Visitor/LODSelectionVisitor.h>
#include <livre/Lib/Configuration/EFPrefetchAlgorithmParameters.h>
#include <livre/core/Render/GenerateRenderingSet.h>
#include <livre/core/Maths/Maths.h>
#include <livre/core/Data/VolumeInformation.h>
#include <livre/core/Render/GLWidget.h>
#include <livre/Lib/Algorithm/Optimizer.h>
#include <livre/Lib/types.h>

namespace livre
{

RenderView::RenderView( )
    : visibleFrustumScreenSpaceError_( 0 )
    , texturePFrustumScreenSpaceError_( 0 )
    , dataPFrustumScreenSpaceError_( 0 )
{
    prefetchDistanceRatios_.resize( 6, 1.0f );
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

bool RenderView::onPreRender_( const GLWidget& /*widget*/,
                               const FrameInfo& frameInfo,
                               GenerateRenderingSet& renderListGenerator,
                               Frustum& modifiedFrustum )
{
    switch( volumeRendererParameters_.renderStrategy )
    {
    default:
        LBUNIMPLEMENTED;
        // no break;
    case RS_ANY_FRAME:
        return true;
    case RS_ITERATED_FULL_FRAME:
        if( !frameInfo.notAvailableRenderNodeList.empty( ))
            generateIteratedFrustum_( renderListGenerator,
                                      frameInfo.previousFrustum,
                                      frameInfo.currentFrustum,
                                      modifiedFrustum );
        return true;
    case RS_FULL_FRAME:
        return frameInfo.notAvailableRenderNodeList.empty();
    }
}

void RenderView::onPostRender_( const bool /*rendered*/,
                                const GLWidget& widget,
                                const FrameInfo& frameInfo,
                                GenerateRenderingSet& renderListGenerator )
{
    if( frameInfo.previousFrustum != frameInfo.currentFrustum )
         freeTextures_( frameInfo.renderNodeList );

    Viewporti pixelViewport;
    widget.setViewport( this, pixelViewport );

    const uint32_t windowHeight = pixelViewport.getHeight();
    generateRequest_( frameInfo.currentFrustum, renderListGenerator, windowHeight );
}

void RenderView::generateIteratedFrustum_( GenerateRenderingSet& renderListGenerator,
                                           const Frustum& previousFrustum,
                                           const Frustum& currentFrustum,
                                           Frustum& modifiedFrustum ) const
{
    Vector3f prevEyePosition;
    Matrix3f previousRotationMat;

    maths::getRotationAndEyePositionFromModelView( previousFrustum.getModelViewMatrix(),
                                                   previousRotationMat,
                                                   prevEyePosition );
    Vector3f newEyePosition;
    Matrix3f newRotationMat;

    maths::getRotationAndEyePositionFromModelView( currentFrustum.getModelViewMatrix(),
                                                   newRotationMat,
                                                   newEyePosition );

    const Quaternionf prevRotation( previousRotationMat );
    const Quaternionf newRotation( newRotationMat );

    const Vector3f difference = newEyePosition - prevEyePosition;

    for( float t = 0.9f; t >= 0.0f ; t+= -0.1f )
    {
        t =  std::max( t, 0.0f );

        Quaternionf interpolatedQuat = Quaternionf::slerp( t, prevRotation, newRotation );
        const Vector3f translation = prevEyePosition + difference * t;

        const Matrix4f& modelViewMatrix = maths::computeModelViewMatrix( interpolatedQuat, translation );

        Frustum iteratedFrustum;
        iteratedFrustum.initialize( modelViewMatrix, currentFrustum.getProjectionMatrix() );

        RenderBricks renderBrickList;
        DashNodeVector allNodeList;
        DashNodeVector renderNodeList;
        DashNodeVector notAvailableRenderNodeList;
        renderListGenerator.generateRenderingSet( iteratedFrustum,
                                                  allNodeList,
                                                  renderNodeList,
                                                  notAvailableRenderNodeList,
                                                  renderBrickList );

        if( notAvailableRenderNodeList.empty() )
        {
            modifiedFrustum = iteratedFrustum;
            return;
        }
    }

    modifiedFrustum = previousFrustum;
}

void RenderView::freeTextures_( const DashNodeVector& renderNodeList )
{
    DashNodeSet currentSet;
    for( DashNodeVector::const_iterator it = renderNodeList.begin(); it != renderNodeList.end(); ++it )
    {
        dash::NodePtr node = *it;
        currentSet.insert( node );
    }

    DashNodeSet cleanSet;
    std::set_difference( previousVisibleSet_.begin( ),
                         previousVisibleSet_.end( ),
                         currentSet.begin( ),
                         currentSet.end( ),
                         std::inserter( cleanSet, cleanSet.begin() ) );

    // Unreference not needed textures
    for( DashNodeSet::iterator it = cleanSet.begin(); it != cleanSet.end(); ++it )
    {
        DashRenderNode renderNode( *it );
        if( renderNode.getLODNode().getRefLevel() != 0 )
            renderNode.setTextureObject( TextureObject::getEmptyPtr() );
    }

    previousVisibleSet_ = currentSet;
}

void RenderView::generateRequest_( const Frustum& currentFrustum,
                                   GenerateRenderingSet& renderListGenerator,
                                   const uint32_t windowHeight )
{

    FloatVector distances;
    distances.resize( 6, 0.0f );

    const VolumeInformation& volumeInfo = renderListGenerator.getVolumeInformation();

    const float wsPerVoxel = volumeInfo.worldSpacePerVoxel;
    const float depth = volumeInfo.depth;
    const float levelZeroNodeSize = float( volumeInfo.maximumBlockSize[ 0 ] ) *
                                    volumeInfo.worldSpacePerVoxel;

    const LODFrustum renderFrustum( currentFrustum,
                                    visibleFrustumScreenSpaceError_,
                                    windowHeight,
                                    wsPerVoxel,
                                    levelZeroNodeSize,
                                    depth,
                                    distances );

    LODSelectionVisitor renderVisitor( renderFrustum, DRT_VISIBILE );
    dash::NodePtr root = renderListGenerator.getDashNodeTree();
    dfsTraverser_.traverse( root, renderVisitor );
}

}
