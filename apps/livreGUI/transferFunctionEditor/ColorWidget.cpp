/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Marwan Abdellah <marwan.abdellah@epfl.ch>
 *                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#include "ColorWidget.h"

#include <QPainter>
#include <QQuickItem>

namespace livre
{

namespace
{
const QUrl QML_ROOT_COMPONENT( "qrc:/qml/ColorWidget.qml" );

QColor getColor( const ControlPointsWidget::Channel channel )
{
    switch( channel )
    {
    case ControlPointsWidget::Channel::red:
        return Qt::red;
    case ControlPointsWidget::Channel::green:
        return Qt::green;
    case ControlPointsWidget::Channel::blue:
    default:
        return Qt::blue;
    }
}
}

ColorWidget::ColorWidget( QWidget* parent_, const Channel channel )
    : ControlPointsWidget( parent_, channel )
{
    setSource( QML_ROOT_COMPONENT );

    rootObject()->setProperty( "color", getColor( channel ));
}

void ColorWidget::resizeEvent( QResizeEvent* ev )
{
    ControlPointsWidget::resizeEvent( ev );

    if( _background.isNull() || _background.size() != ev->size( ))
    {
        _background = QImage( size(), QImage::Format_RGB32 );

        QLinearGradient gradient( 0, 0, 0, height( ));
        gradient.setColorAt( 1, Qt::black );
        gradient.setColorAt( 0, getColor( getChannel( )));

        QPainter painter( &_background );
        painter.fillRect( rect(), gradient );
    }
}

}
