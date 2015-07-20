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

void View::setViewport( const Viewport& viewport )
{
    viewport_ = viewport;
}

RendererPtr View::getRenderer() const
{
    return rendererPtr_;
}

const Viewport& View::getViewport() const
{
    return viewport_;
}

void View::render( const FrameInfo& frameInfo,
                   const RenderBricks& renderBricks,
                   const GLWidget& widget )
{
    if( !rendererPtr_ )
        return ;

#ifndef LIVRE_DEBUG_RENDERING
    const size_t all = frameInfo.allNodesList.size();
    const size_t haveNot = frameInfo.notAvailableRenderNodeList.size();
    const size_t have = all - haveNot;

    if( progress_.expected_count() != all )
        progress_.restart( all );
    progress_ += static_cast< unsigned long >( have ) - progress_.count();
#endif

    onPreRender_( widget, frameInfo );

     if( !renderBricks.empty( ))
        rendererPtr_->render( widget,
                              *this,
                              frameInfo.currentFrustum,
                              renderBricks );

    onPostRender_( widget, frameInfo );
}

FrameInfo::FrameInfo( const Frustum& cFrustum )
    : currentFrustum( cFrustum )
{
}

}
