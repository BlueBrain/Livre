/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Marwan Abdellah <marwan.abdellah@epfl.ch>
 *                     Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#include <livreGUI/editor/GradientRenderer.h>

#include <QtWidgets>

namespace livre
{

GradientRenderer::GradientRenderer( QWidget* grParentWidget )
    : TFWidget( grParentWidget )
{
    _createCheckersBackground();
}

void GradientRenderer::setGradientStops( const QGradientStops &stops )
{
    _gradient = QLinearGradient( 0.0f, 0.0f, width(), 0.0f );

    for ( int i = 0; i < stops.size(); ++i )
    {
        _gradient.setColorAt( stops.at( i ).first, stops.at( i ).second );
    }

    _shade = QImage();
    _generateShade();
    update();
}

void GradientRenderer::paintEvent( QPaintEvent* )
{
    _generateShade();

    QPainter painter( this );
    painter.drawImage( 0, 0, _shade );

    painter.setPen( QColor( 255, 255, 255 ));
    painter.drawRect( 0, 0, width(), height());
}

void GradientRenderer::_generateShade()
{
    if ( _shade.isNull() || _shade.size() != size())
    {

        _shade = QImage( size(), QImage::Format_ARGB32_Premultiplied );
        _shade.fill( 0 );

        QPainter painter( &_shade );
        painter.fillRect( rect(), _gradient );
    }
}

QSize GradientRenderer::sizeHint() const
{
    return QSize( 255, 255 );
}

}
