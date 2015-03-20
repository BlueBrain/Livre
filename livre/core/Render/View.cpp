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

#include <livre/core/Render/View.h>
#include <livre/core/Render/Renderer.h>
#include <livre/core/Render/RenderingSetGenerator.h>
#include <livre/core/Render/GLWidget.h>

namespace livre
{

View::View()
    : progress_( 8 ) // 8: expected # of bricks
{
}

View::~View()
{
}

void View::setRenderer( RendererPtr rendererPtr )
{
    rendererPtr_ = rendererPtr;
}

void View::setViewport( const Viewportf& viewport )
{
    viewport_ = viewport;
}

RendererPtr View::getRenderer() const
{
    return rendererPtr_;
}

const Viewportf& View::getViewport() const
{
    return viewport_;
}

void View::render( const GLWidget& widget,
                   RenderingSetGenerator& renderListGenerator )
{
    if( !rendererPtr_ )
        return ;

    previousFrustum_ = currentFrustum_;
    currentFrustum_ = getFrustum();

    FrameInfo frameInfo( currentFrustum_, previousFrustum_ );

    renderListGenerator.generateRenderingSet( currentFrustum_,
                                              frameInfo.allNodeList,
                                              frameInfo.renderNodeList,
                                              frameInfo.notAvailableRenderNodeList,
                                              frameInfo.renderBrickList );
#ifndef LIVRE_DEBUG_RENDERING
    const size_t all = frameInfo.allNodeList.size();
    const size_t haveNot = frameInfo.notAvailableRenderNodeList.size();
    const size_t have = all - haveNot;

    if( progress_.expected_count() != all )
        progress_.restart( all );
    progress_ += static_cast< unsigned long >( have ) - progress_.count();
#endif

    Frustum newFrustum = currentFrustum_;

    const bool continueRendering = onPreRender_( widget, frameInfo,
                                                 renderListGenerator,
                                                 newFrustum );
    if( continueRendering )
    {
        currentFrustum_ = newFrustum;

        if( !frameInfo.renderBrickList.empty( ))
            rendererPtr_->render( widget, *this, currentFrustum_,
                                  frameInfo.renderBrickList );
    }

    onPostRender_( continueRendering, widget, frameInfo, renderListGenerator );
}

bool View::onPreRender_( const GLWidget& glWidget LB_UNUSED,
                         const FrameInfo& frameInfo LB_UNUSED,
                         RenderingSetGenerator& renderListGenerator LB_UNUSED,
                         Frustum& modifiedFrustum LB_UNUSED )
{
    return true;
}

void View::onPostRender_( const bool rendered LB_UNUSED,
                          const GLWidget& glWidget LB_UNUSED,
                          const FrameInfo& frameInfo LB_UNUSED,
                          RenderingSetGenerator& renderListGenerator LB_UNUSED )
{
}

FrameInfo::FrameInfo( const Frustum& cFrustum, const Frustum& pFrustum )
    : previousFrustum( pFrustum ),
      currentFrustum( cFrustum )
{
}

}
