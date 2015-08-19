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

#include <livreGUI/editor/TFWidget.h>

#include <QtWidgets>

namespace livre
{

TFWidget::TFWidget( QWidget* grParentWidget )
    : QWidget( grParentWidget )
    , _gradient( QLinearGradient( 0, 0, 0, 0 ))
{}

void TFWidget::_createCheckersBackground()
{
    QPixmap pixmap( 20, 20 );
    QPainter pixmapPainter( &pixmap );
    pixmapPainter.fillRect( 0, 0, 10, 10, Qt::lightGray );
    pixmapPainter.fillRect( 10, 10, 10, 10, Qt::lightGray );
    pixmapPainter.fillRect( 0, 10, 10, 10, Qt::darkGray );
    pixmapPainter.fillRect( 10, 0, 10, 10, Qt::darkGray );
    pixmapPainter.end();
    QPalette colorPalette = palette();
    colorPalette.setBrush( backgroundRole(), QBrush( pixmap ));
    setAutoFillBackground( true );
    setPalette( colorPalette );
}

}
