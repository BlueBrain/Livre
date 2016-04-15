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

#include <livreGUI/transferFunctionEditor/ColorMapWidget.h>
#include <livreGUI/transferFunctionEditor/HoverPoints.h>

namespace livre
{

ColorMapWidget::ColorMapWidget( const ShadeType type, QWidget* parent_ )
    : TFWidget( parent_ )
    , _shadeType( type )
{
    // Add checkers background for the alpha widget.
    _shadeType == ARGB_SHADE ? _createCheckersBackground()
                             : setAttribute( Qt::WA_NoBackground );

    QPolygonF points;
    points << QPointF( 0, sizeHint().height())
           << QPointF( sizeHint().width(), 0 );

    _hoverPoints = new HoverPoints( this, HoverPoints::CIRCLE_POINT );
    _hoverPoints->setPoints( points );
    _hoverPoints->setConnectionType( HoverPoints::LINE_CONNECTION );
    _hoverPoints->setPointLock( 0, HoverPoints::LOCK_TO_LEFT );
    _hoverPoints->setPointLock( 1, HoverPoints::LOCK_TO_RIGHT );
    _hoverPoints->setSortType( HoverPoints::X_SORT );

    connect( _hoverPoints, SIGNAL( pointsChanged( QPolygonF )),
             this, SIGNAL( colorsChanged()));
}

QPolygonF ColorMapWidget::getPoints() const
{
    return _hoverPoints->points();
}

uint32_t ColorMapWidget::getColorAtPoint( const int32_t xPosition )
{
    _generateShade();

    const QPolygonF& hoverPoints = _hoverPoints->points();
    for( int i = 1; i < hoverPoints.size(); ++i )
    {
        if( hoverPoints.at(i - 1).x() > xPosition ||
            hoverPoints.at(i).x() < xPosition )
        {
            continue;
        }

        QLineF line( hoverPoints.at( i-1 ), hoverPoints.at( i ) );
        line.setLength(( xPosition - line.x1( )) / line.dx() * line.length( ));

        if( _shadeType != ARGB_SHADE )
            return _shade.pixel( qRound( qMin( line.x2(),
                                               qreal( _shade.width() - 1 ))),
                                 qRound( qMin( line.y2(),
                                               qreal( _shade.height() - 1 ))));

        const float alpha = std::min( 1.f,
                                      float( line.y2( )) /
                                      float( _shade.height() - 1 ));
        const uint32_t pixel =
            _shade.pixel( qRound( qMin( line.x2(),
                                        qreal( _shade.width() - 1 ))), 0 );
        return ( pixel & 0xffffffu ) |
            ( qMin( unsigned( (1.f - alpha) * 255 ), 255u ) << 24u );
    }
    return 0u;
}

UInt8s ColorMapWidget::getCurve() const
{
    UInt8s curve;

    int32_t currentHPointIndex = 0;
    const size_t tfSize = 256u;
    const float scale = float( width()) / 255.0f;

    for( size_t i = 0; i < tfSize; ++i )
    {
        const float realX = i * scale;
        if( _hoverPoints->points().at( currentHPointIndex + 1 ).x() < realX )
            currentHPointIndex++;

        const QLineF currentLine( _hoverPoints->points().at( currentHPointIndex ),
                                  _hoverPoints->points().at( currentHPointIndex + 1 ));
        const float slope = currentLine.dy() / currentLine.dx();
        const float realY = ( realX - currentLine.p1().x()) * slope + currentLine.p1().y();

        const uint8_t currentCurveValue = 255u * ( 1.0f - realY / height());
        curve.push_back( currentCurveValue );
    }
    return curve;
}

void ColorMapWidget::setGradientStops( const QGradientStops& stops )
{
    if( _shadeType != ARGB_SHADE )
        return;

    _gradient = QLinearGradient( 0.0f, 0.0f, width(), 0.0f );

    for( int i = 0; i < stops.size(); ++i )
        _gradient.setColorAt( stops.at( i ).first, stops.at( i ).second );

    _shade = QImage();
    _generateShade();
    update();
}

void ColorMapWidget::paintEvent( QPaintEvent* )
{
    _generateShade();

    QPainter painter( this );
    painter.drawImage( 0, 0, _shade );

    painter.setPen( QColor( 255, 255, 255 ));
    painter.drawRect( 0, 0, width(), height());
}

void ColorMapWidget::_generateShade()
{
    if( _shade.isNull() || _shade.size() != size( ))
    {
        // Alpha widget
        if( _shadeType == ARGB_SHADE )
        {
            _shade = QImage( size(), QImage::Format_ARGB32_Premultiplied );
            _shade.fill( 0 );
            QPainter painter( &_shade );
            painter.fillRect( rect(), _gradient );
        }
        else // RGB widgets
        {
            _shade = QImage( size(), QImage::Format_RGB32 );
            QLinearGradient shade( 0, 0, 0, height());

            shade.setColorAt( 1, Qt::black );

            if( _shadeType == RED_SHADE )
                shade.setColorAt( 0, Qt::red );
            else if ( _shadeType == GREEN_SHADE )
                shade.setColorAt( 0, Qt::green );
            else
                shade.setColorAt( 0, Qt::blue );

            QPainter painter( &_shade );
            painter.fillRect( rect(), shade );
        }
    }
}

HoverPoints* ColorMapWidget::getHoverPoints() const
{
    return _hoverPoints;
}

QSize ColorMapWidget::sizeHint() const
{
    return QSize( 255, 255 );
}

void ColorMapWidget::setPoints( const QPolygonF &points )
{
    _hoverPoints->setPoints( points );
    _hoverPoints->setPointLock( 0, HoverPoints::LOCK_TO_LEFT );
    _hoverPoints->setPointLock( points.size() - 1, HoverPoints::LOCK_TO_RIGHT );
    update();
}

}
