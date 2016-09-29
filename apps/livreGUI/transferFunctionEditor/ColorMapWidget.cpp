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

ColorMapWidget::ColorMapWidget( QWidget* parent_,
                                lexis::render::ColorMap& colorMap,
                                const lexis::render::ColorMap::Channel channel )
    : QWidget( parent_ )
    , _colorMap( colorMap )
    , _channel( channel )
    , _isLogScale( false )
    , _gradient( QLinearGradient( 0, 0, 0, 0 ))
{
    setMouseTracking( true );

    if( _channel == lexis::render::ColorMap::Channel::alpha )
        _createCheckersBackground();
    else
        setAttribute( Qt::WA_NoBackground );

    QPolygonF points;
    points << QPointF( 0, sizeHint().height())
           << QPointF( sizeHint().width(), 0 );

    _hoverPoints = new HoverPoints( this, colorMap, channel );
    connect( _hoverPoints, SIGNAL( pointsChanged( )), this, SIGNAL( colorsChanged()));
}

void ColorMapWidget::mouseMoveEvent( QMouseEvent* mouseEvent )
{
    if( _channel != lexis::render::ColorMap::Channel::alpha || _histogram.isEmpty( ))
        return;

    const size_t index = (float)_histogram.getBins().size() *
                         (float)mouseEvent->pos().x() / (float)width();
    emit histIndexChanged( index, _histogram.getRatio( index ));
}

void ColorMapWidget::leaveEvent( QEvent* )
{
    if( _channel != lexis::render::ColorMap::Channel::alpha )
        return;

    emit histIndexChanged( -1u, 0.0f );
}

void ColorMapWidget::setGradientStops( const QGradientStops& stops )
{
    if( _channel != lexis::render::ColorMap::Channel::alpha )
        return;

    _gradient = QLinearGradient( 0.0f, 0.0f, width(), 0.0f );

    for( int i = 0; i < stops.size(); ++i )
        _gradient.setColorAt( stops.at( i ).first, stops.at( i ).second );

    _background = QImage();
    _generateBackground();
}

void ColorMapWidget::setHistogram( const Histogram& histogram, const bool isLogScale )
{
    if( _channel != lexis::render::ColorMap::Channel::alpha )
        return;

    _histogram = histogram;
    _isLogScale = isLogScale;
}

void ColorMapWidget::paintEvent( QPaintEvent* )
{
    _generateBackground();
    _drawHistogram();

    QPainter painter( this );
    painter.drawImage( 0, 0, _background );

    painter.setPen( QColor( 255, 255, 255 ));
    painter.drawRect( 0, 0, width(), height());
}

void ColorMapWidget::_generateBackground()
{
    if( !_background.isNull() && _background.size() == size( ))
        return;

    _background = QImage( size(), _channel == lexis::render::ColorMap::Channel::alpha
                             ? QImage::Format_ARGB32_Premultiplied
                             : QImage::Format_RGB32 );

    // Alpha widget
    if( _channel == lexis::render::ColorMap::Channel::alpha )
    {
        _background.fill( 0 );
        QPainter painter( &_background );
        painter.fillRect( _background.rect(), _gradient );
        return;
    }

    QLinearGradient gradient( 0, 0, 0, height( ));
    gradient.setColorAt( 1, Qt::black );

    if( _channel == lexis::render::ColorMap::Channel::red )
        gradient.setColorAt( 0, Qt::red );
    else if ( _channel == lexis::render::ColorMap::Channel::green )
        gradient.setColorAt( 0, Qt::green );
    else
        gradient.setColorAt( 0, Qt::blue );

    QPainter painter( &_background );
    painter.fillRect( rect(), gradient );
}

void ColorMapWidget::_drawHistogram()
{
    if( _channel != lexis::render::ColorMap::Channel::alpha || _background.isNull() || _histogram.isEmpty( ))
        return;

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

QSize ColorMapWidget::sizeHint() const
{
    return QSize( 255, 255 );
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


}
