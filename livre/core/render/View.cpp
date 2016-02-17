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

namespace livre
{

View::View()
{
}

View::~View()
{
}

void View::setRenderer( RendererPtr renderer )
{
    _renderer = renderer;
}

void View::setViewport( const Viewport& viewport )
{
    _viewport = viewport;
}

RendererPtr View::getRenderer() const
{
    return _renderer;
}

const Viewport& View::getViewport() const
{
    return _viewport;
}

void View::render( const FrameInfo& frameInfo,
                   const RenderBricks& bricks,
                   const GLWidget& widget )
{
    if( !_renderer )
        return ;

     if( !bricks.empty( ))
        _renderer->render( widget, *this, bricks );

    _onPostRender( widget, frameInfo );
}
}
