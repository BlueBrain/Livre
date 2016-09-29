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

#include <livreGUI/transferFunctionEditor/HoverPoints.h>

#include <iostream>

namespace livre
{

namespace
{
const float controlPointSize = 8.0f;
const QPointF emptyPoint( -1.0f, -1.0f );
const QPen pointPen( QColor( 255, 255, 255, 191 ), 1 );
const QPen connectionPen( QPen( QColor( 255, 255, 255, 127 ), 2 ));
QBrush pointBrush( QColor( 191, 191, 191, 127 ));
QSize pointSize( 22, 22 );
}

HoverPoints::HoverPoints( QWidget *widget,
                          lexis::render::ColorMap& colorMap,
                          const lexis::render::ColorMap::Channel channel )
    : QObject( widget )
    , _colorMapWidget( widget )
    , _editable( true )
    , _enabled( true )
    , _selectedPoint( emptyPoint )
    , _colorMap( colorMap )
    , _channel( channel )
{
    widget->installEventFilter( this );
    widget->setAttribute( Qt::WA_AcceptTouchEvents );
    connect( this, SIGNAL( pointsChanged()), _colorMapWidget, SLOT( update()));
}

void HoverPoints::setEnabled( bool enabled )
{
    if( _enabled != enabled )
    {
        _enabled = enabled;
        _colorMapWidget->update();
    }
}

bool HoverPoints::eventFilter( QObject* object, QEvent* hoverEvent )
{
    // If the selected object is the transfer object and the widget is enabled.
    if( object != _colorMapWidget || !_enabled || !_editable )
        return false;

    auto& controlPoints = _colorMap.getControlPoints( _channel );
    if( controlPoints.empty( ))
        return false;

    const float w = _colorMapWidget->width();
    const float h = _colorMapWidget->height();

    // Detect the event type.
    switch ( hoverEvent->type())
    {
    case QEvent::MouseButtonPress:
    {
        QMouseEvent* mouseEvent = (QMouseEvent*)hoverEvent;
        QPointF clickPosition = mouseEvent->pos();
        QPointF selectedPoint = emptyPoint;
        for( size_t i = 0; i < controlPoints.size(); ++i )
        {
            // Select the shape of the bounding rectangle of the volume
            // whether it is circle of rectangle.
            QPainterPath touchSurface;
            const auto& cp = controlPoints[ i ];
            const QPointF point( cp.getX() / 256.0f * w,
                                 ( 1.0f - cp.getY( )) * h );
            touchSurface.addEllipse( point, controlPointSize, controlPointSize );

            // If the mouse event was applied in this boundary of the point,
            // set the index to that of the selected point.
            if( touchSurface.contains( clickPosition ))
            {
                selectedPoint = { cp.getX(), cp.getY() };
                break;
            }
        }

        // If the Qt::LeftButton is clicked where there are no points,
        // insert a new point, and if the Qt::LeftButton is clicked where
        // a point already exists, then move the HoverPoint.
        if( mouseEvent->button() == Qt::LeftButton )
        {
            // If there is no point where the mouse is clicked, then create
            // a new point.
            if( selectedPoint == emptyPoint )
            {
                lexis::render::ControlPoint cp(
                            (float)clickPosition.x() / w * 256.0f,
                            1.0f - (float)clickPosition.y() / h );

                // Insert a new point at this position.
                _colorMap.addControlPoint( cp,  _channel );
                _selectedPoint = QPointF( cp.getX(), cp.getY( ));
                // Select the point

                // Update the system.
                emit pointsChanged();
            }
            else // If there is a specific point that is clicked, get it.
                _selectedPoint = selectedPoint;

            // We have created or selected a point.
            return true;

        }
        // If the Qt::RightButton is selcted where there is a point, then
        // delete this point and update the system.
        else if( mouseEvent->button() == Qt::RightButton )
        {
            // If there is a specified point that is selected based on
            // the index and the widget is editible.
            if( selectedPoint != emptyPoint )
            {
                const float selectedX = selectedPoint.x();

                // If it is the last point or the first point, do not remove
                if(  selectedX == controlPoints[ 0 ].getX() ||
                     selectedX == controlPoints[ controlPoints.size() - 1 ].getX( ))
                {
                    return true;
                }

                _colorMap.removeControlPoint( selectedX, _channel );

                // Update the system.
                emit pointsChanged();

                // We have deleted a point.
                return true;
            }
        }
    }   break;

    case QEvent::MouseButtonRelease:
    {
        _selectedPoint = emptyPoint;
    }   break;

    case QEvent::MouseMove:
    {
        // If there is a point selected with a specific index, move it.
        if( _selectedPoint == emptyPoint )
            return false;

        const auto& pos = ((QMouseEvent*)hoverEvent )->pos();
        const float y = std::max( std::min( 1.0f, 1.0f - pos.y() / h ), 0.0f );
        const float selectedX = _selectedPoint.x();

        // If it is the last point or the first point
        if(  selectedX == controlPoints[ 0 ].getX() ||
             selectedX == controlPoints[ controlPoints.size() - 1 ].getX( ))
        {
            // Update the control point only on y direction
            _colorMap.addControlPoint( { selectedX, y }, _channel );
            emit pointsChanged();
            return true;
        }

        const float x = pos.x() / w * 256.0f;

        // If position reaches or passes the boundaries
        if( x <= controlPoints[ 0 ].getX( )
            || x >= controlPoints[ controlPoints.size() - 1 ].getX( ))
        {
            return true;
        }

        // If there is previously a control point in the position
        for( size_t i = 0; i < controlPoints.size(); ++i )
        {
            if( controlPoints[ i ].getX() == x && _selectedPoint.x() != x )
                return true;
        }

        _colorMap.removeControlPoint( selectedX, _channel );
        _colorMap.addControlPoint( { x, y }, _channel );
        _selectedPoint = { x, y };

        // Update the system.
        emit pointsChanged();
        return true;

    }   break;

    case QEvent::Resize: // Resize the points.
    {
        // Update the system.
        emit pointsChanged();
        break;
    }

    case QEvent::Paint: // Render the points on the widget.
    {
        QWidget* tfWidget = _colorMapWidget;
        _colorMapWidget = 0;

        QApplication::sendEvent( object, hoverEvent );
        _colorMapWidget = tfWidget;

        // Paing all the points on the widget.
        paintPoints();
        return true;
    }
    default: break;
    }

    // No event is selected.
    return false;
}

void HoverPoints::paintPoints()
{
    QPainter qPainter;
    qPainter.begin( _colorMapWidget );

    // Set anti-aliasing.
    qPainter.setRenderHint( QPainter::Antialiasing );

    QPolygonF polygon;
    const auto& controlPoints = _colorMap.getControlPoints( _channel );

    const float w = _colorMapWidget->width();
    const float h = _colorMapWidget->height();

    for( size_t i = 0; i < controlPoints.size(); ++i )
    {
        polygon << QPointF( controlPoints[ i ].getX() / 256.0f * w,
                            ( 1.0f - controlPoints[ i ].getY()) * h );

    }

    qPainter.setPen( connectionPen );
    qPainter.drawPolyline( polygon );

    // Set the style.
    qPainter.setPen( pointPen );
    qPainter.setBrush( pointBrush );

    // After drawing the lines between the points, draw the points' shapes.
    for( const auto& pnt: polygon )
        qPainter.drawEllipse( pnt, controlPointSize, controlPointSize );
}

void HoverPoints::setEditable( const bool editable )
{
    _editable = editable;
}

bool HoverPoints::isEditable() const
{
    return _editable;
}

}
