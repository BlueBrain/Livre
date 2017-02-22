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

#include "ColorMapWidget.h"
#include "HoverPoints.h"

#include <QMouseEvent>
#include <QPainter>

#include <cmath>

namespace livre
{

ColorMapWidget::ColorMapWidget( QWidget* parent_, const Channel channel )
    : QWidget( parent_ )
    , _channel( channel )
{
    setMouseTracking( true );

    if( _channel == Channel::alpha )
        _createCheckersBackground();
    else
        setAttribute( Qt::WA_NoBackground );

    _hoverPoints = new HoverPoints( this );
    connect( _hoverPoints, SIGNAL( pointsChanged( )),
             this, SIGNAL( colorsChanged()));
}

QPolygonF ColorMapWidget::getControlPoints() const
{
    return _hoverPoints->getControlPoints();
}

uint32_t ColorMapWidget::getColorAtPoint( const float xPosition ) const
{
    const auto& controlPoints = _hoverPoints->getControlPoints();
    for( int i = 1; i < controlPoints.size(); ++i )
    {
        auto p1 = controlPoints.at(i - 1);
        auto p2 = controlPoints.at(i);

        if( p1.x() > xPosition || p2.x() < xPosition )
            continue;

        // transform normalized to onscreen absolute size for background pixel lookup
        p1.setX( p1.x() * width() );
        p1.setY( (1.f-p1.y()) * height());
        p2.setX( p2.x() * width() );
        p2.setY( (1.f-p2.y()) * height());

        QLineF line( p1, p2 );
        line.setLength(( (xPosition*width()) - line.x1( )) / line.dx() * line.length( ));

        if( _channel != Channel::alpha )
            return _background.pixel( qRound( qMin( line.x2(),
                                               qreal( width() - 1 ))),
                                 qRound( qMin( line.y2(),
                                               qreal( height() - 1 ))));

        const float alpha = std::min( 1.f,
                                      float( line.y2( )) /
                                      float( height() - 1 ));
        const uint32_t pixel =
            _background.pixel( qRound( qMin( line.x2(),
                                        qreal( width() - 1 ))), 0 );
        return ( pixel & 0xffffffu ) |
            ( qMin( unsigned( (1.f - alpha) * 255 ), 255u ) << 24u );
    }
    return 0u;
}

UInt8s ColorMapWidget::getCurve() const
{
    size_t currentHPointIndex = 0;

    const auto& controlPoints = _hoverPoints->getControlPoints();
    UInt8s curve;
    curve.reserve( COLORSAMPLES );
    for( size_t i = 0; i < COLORSAMPLES; ++i )
    {
        float normX = i / float(COLORSAMPLES-1);
        if( controlPoints.at( currentHPointIndex + 1 ).x() < normX )
            currentHPointIndex++;

        const QLineF currentLine( controlPoints.at( currentHPointIndex ),
                                  controlPoints.at( currentHPointIndex + 1 ));
        const float slope = currentLine.dy() / currentLine.dx();
        const float normY = ( normX - currentLine.p1().x()) * slope + currentLine.p1().y();

        const uint8_t currentCurveValue = 255u * normY;
        curve.push_back( currentCurveValue );
    }
    return curve;
}

void ColorMapWidget::mouseMoveEvent( QMouseEvent* mouseEvent )
{
    if( _channel != Channel::alpha || _histogram.isEmpty( ))
        return;

    const size_t index = (float)_histogram.getBins().size() *
                         (float)mouseEvent->pos().x() / (float)width();
    emit histIndexChanged( index, _histogram.getRatio( index ));
}

void ColorMapWidget::leaveEvent( QEvent* )
{
    if( _channel != Channel::alpha )
        return;

    emit histIndexChanged( -1u, 0.0f );
}

void ColorMapWidget::setGradientStops( const QGradientStops& stops )
{
    _gradient = QLinearGradient( 0.0f, 0.0f, width(), 0.0f );

    for( int i = 0; i < stops.size(); ++i )
        _gradient.setColorAt( stops.at( i ).first, stops.at( i ).second );

    _generateBackground();
    update();
}

void ColorMapWidget::setHistogram( const lexis::render::Histogram& histogram,
                                   const bool isLogScale )
{
    _histogram = histogram;
    _isLogScale = isLogScale;
    _generateBackground();
    update();
}

void ColorMapWidget::paintEvent( QPaintEvent* )
{
    QPainter painter( this );
    painter.drawImage( 0, 0, _background );

    painter.setPen( QColor( 255, 255, 255 ));
    painter.drawRect( 0, 0, width(), height());
}

void ColorMapWidget::resizeEvent( QResizeEvent* ev )
{
    QWidget::resizeEvent( ev );

    if( _background.isNull() || _background.size() != ev->size( ))
        _generateBackground();
}

void ColorMapWidget::_generateBackground()
{
    _background = QImage( size(), _channel == Channel::alpha
                             ? QImage::Format_ARGB32_Premultiplied
                             : QImage::Format_RGB32 );

    if( _channel == Channel::alpha )
    {
        _gradient.setFinalStop( width(), 0 );
        _background.fill( 0 );

        {
            QPainter painter( &_background );
            painter.fillRect( _background.rect(), _gradient );
        }

        if( !_histogram.isEmpty( ))
            _drawHistogram();

        return;
    }

    QLinearGradient gradient( 0, 0, 0, height( ));
    gradient.setColorAt( 1, Qt::black );

    if( _channel == Channel::red )
        gradient.setColorAt( 0, Qt::red );
    else if ( _channel == Channel::green )
        gradient.setColorAt( 0, Qt::green );
    else
        gradient.setColorAt( 0, Qt::blue );

    QPainter painter( &_background );
    painter.fillRect( rect(), gradient );
}

void ColorMapWidget::_drawHistogram()
{
    const QRect viewPort = rect();
    const int xLeft = viewPort.left();
    const int xRight = viewPort.right();
    const int yBottom = viewPort.bottom();
    const int xWidth = viewPort.width();
    const int xHeight = viewPort.height();

    const size_t nbBins = _histogram.getBins().size();
    const uint64_t* bins = _histogram.getBins().data();
    // Find maximum height in bins unit
    const uint64_t heightMax = bins[ _histogram.getMaxIndex( )];
    if( heightMax == 0 )
        return;

    const float wScale = float( nbBins ) / float( xWidth );
    const float hScale =  xHeight * 0.98f / ( _isLogScale ? std::log( float( heightMax ))
                                                 : float( heightMax ));
    QPen pen;
    QPainterPath path;
    pen.setColor( Qt::black );
    pen.setWidth( 2 );
    QPainter painter( &_background );
    painter.setRenderHint( QPainter::Antialiasing );
    painter.setPen( pen );
    pen.setStyle( Qt::SolidLine );
    path.moveTo( QPoint( float( xLeft ) + 0.5f, yBottom ));
    for( int i = 0; i < xWidth; ++i )
    {
        float value = bins[ size_t( wScale * i ) ];
        if( _isLogScale && value <= 1.0f )
            value = 1.0f;
        path.lineTo( QPoint( float( xLeft + i ) + 0.5,
                           yBottom - hScale * ( _isLogScale ? std::log( value ) : value )));
    }
    path.lineTo( QPoint( float( xRight ) + 0.5f, yBottom ));
    painter.drawPath( path );
}

void ColorMapWidget::_createCheckersBackground()
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

void ColorMapWidget::setControlPoints( const QPolygonF& controlPoints )
{
    _hoverPoints->setControlPoints( controlPoints );
    update();
}

}
