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

#include <livreGUI/editor/ColorMapWidget.h>
#include <livreGUI/editor/HoverPoints.h>

namespace livre
{

ColorMapWidget::ColorMapWidget( const ShadeType type, QWidget* colorMapParentWidget )
    : TFWidget( colorMapParentWidget )
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

uint32_t ColorMapWidget::getColorAtPoint( const int xPosition )
{
    _generateShade();

    const QPolygonF& hoverPoints = _hoverPoints->points();
    for ( int32_t i = 1; i < hoverPoints.size(); ++i )
    {
        if( hoverPoints.at(i - 1).x() <= xPosition && hoverPoints.at(i).x() >= xPosition )
        {
            QLineF pointsConnection( hoverPoints.at( i-1 ), hoverPoints.at( i ) );
            const float length = pointsConnection.length() *
                    ( xPosition - pointsConnection.x1()) / pointsConnection.dx();
            pointsConnection.setLength( length );

            return _shade.pixel( qRound( qMin( pointsConnection.x2(),
                                            ( qreal(_shade.width() - 1 )))),
                                 qRound( qMin( pointsConnection.y2(),
                                              qreal( _shade.height() - 1 ))));
        }
    }
    return 0u;
}

UInt8Vector ColorMapWidget::getCurve()
{
    UInt8Vector curve;

    const float delta = 256.0f / float( width() );
    for( int32_t i = 0; i < _hoverPoints->points().size(); ++i )
    {
        const QLineF currentLine( _hoverPoints->points().at( i ), _hoverPoints->points().at( i + 1 ));

        const uint32_t beginPos = currentLine.p1().x() * delta + 0.5f;
        const uint32_t endPos = currentLine.p2().x() * delta + 0.5f;

        for( uint32_t j = beginPos; j < endPos; ++j )
        {
            const float slope = currentLine.dy() / currentLine.dx();
            const float realX = j / delta;
            const float realY = ( realX - currentLine.p1().x()) * slope + currentLine.p1().y();

            const uint8_t currentCurveValue = 255u * ( 1.0f - realY / height());
            curve.push_back( currentCurveValue );
        }
    }
    return curve;
}

void ColorMapWidget::setGradientStops( const QGradientStops& stops )
{
    if( _shadeType == ARGB_SHADE )
    {
        _gradient = QLinearGradient( 0.0f, 0.0f, width(), 0.0f );

        for( int32_t i = 0; i < stops.size(); ++i )
        {
            const QColor& color = stops.at( i ).second;
            _gradient.setColorAt( stops.at( i ).first, QColor( color.red(), color.green(),
                                                               color.blue()));
        }
        _shade = QImage();
        _generateShade();
        update();
    }
}

void ColorMapWidget::paintEvent( QPaintEvent* )
{
    _generateShade();

    QPainter painter( this );
    painter.drawImage( 0, 0, _shade );

    painter.setPen( QColor( 255, 255, 255 ) );
    painter.drawRect( 0, 0, width(), height());
}

void ColorMapWidget::_generateShade()
{
    if( _shade.isNull() || _shade.size() != size() )
    {
        // Alpha widget
        if( _shadeType == ARGB_SHADE )
        {
            _shade = QImage( size(), QImage::Format_ARGB32_Premultiplied );
            _shade.fill(0);

            QPainter painter( &_shade );
            painter.fillRect( rect(), _gradient );

            painter.setCompositionMode( QPainter::CompositionMode_DestinationIn );
            QLinearGradient fade( 0, 0, 0, height());
            fade.setColorAt( 0, QColor(0, 0, 0, 255 ));
            fade.setColorAt( 1, QColor(0, 0, 0, 0 ));
            painter.fillRect( rect(), fade );
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
