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

#include <livre/core/render/View.h>
#include <livre/core/render/FrameInfo.h>
#include <livre/core/render/GLWidget.h>
#include <livre/core/render/Renderer.h>
#include <livre/core/render/RenderingSetGenerator.h>

namespace livre
{

View::View()
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

void View::render( const FrameInfo& frameInfo, const RenderBricks& bricks,
                   const GLWidget& widget )
{
    if( !rendererPtr_ )
        return ;

    onPreRender_( widget, frameInfo );

     if( !bricks.empty( ))
        rendererPtr_->render( widget, *this, frameInfo.currentFrustum, bricks );

    onPostRender_( widget, frameInfo );
}
}
