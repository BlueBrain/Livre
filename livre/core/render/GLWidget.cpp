/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#include <livre/core/render/GLWidget.h>
#include <livre/core/render/View.h>

namespace livre
{

GLWidget::GLWidget()
    : GLContextTrait( GLContextPtr( ))
{
}

GLWidget::~GLWidget( )
{

}

Viewport GLWidget::getViewport( const View& view  ) const
{
    const Vector2f window( getWidth(), getHeight() );
    const Vector2i& pos = view.getViewport().get_sub_vector< 2 >() * window;
    const Vector2i& size = view.getViewport().get_sub_vector< 2, 2 >() * window;

    return Viewport( pos.x(), pos.y(), size.x(), size.y( ));
}

}
