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

#include "HoverPoints.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

namespace livre
{

namespace
{
const float controlPointSize = 8.0f;
const QPointF emptyPoint( -1.0f, -1.0f );
const QPen pointPen( QColor( 255, 255, 255, 191 ), 1 );
const QPen connectionPen( QPen( QColor( 255, 255, 255, 127 ), 2 ));
QBrush pointBrush( QColor( 191, 191, 191, 127 ));
}

HoverPoints::HoverPoints( QWidget* widget )
    : QObject( widget )
    , _colorMapWidget( widget )
    , _selectedPoint( emptyPoint )
{
    widget->installEventFilter( this );
    widget->setAttribute( Qt::WA_AcceptTouchEvents );
    connect( this, SIGNAL( pointsChanged()), _colorMapWidget, SLOT( update()));
}

bool HoverPoints::eventFilter( QObject* object, QEvent* hoverEvent )
{
    if( object != _colorMapWidget )
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
        for( const auto& cp : _controlPoints )
        {
            // Select the shape of the bounding rectangle of the volume
            // whether it is circle of rectangle.
            QPainterPath touchSurface;
            const QPointF point( cp.x() * w,
                                 ( 1.0f - cp.y( )) * h );
            touchSurface.addEllipse( point, controlPointSize, controlPointSize );

            // If the mouse event was applied in this boundary of the point,
            // set the index to that of the selected point.
            if( touchSurface.contains( clickPosition ))
            {
                selectedPoint = { cp.x(), cp.y() };
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
                // Insert a new point at this position.
                _selectedPoint = QPointF( (float)clickPosition.x() / w,
                                          1.0f - (float)clickPosition.y() / h);

                _controlPoints.insert( _selectedPoint );

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
                if( selectedX == _controlPoints.begin()->x() ||
                    selectedX == _controlPoints.rbegin()->x( ))
                {
                    return true;
                }

                _controlPoints.erase( selectedPoint );

                // Update the system.
                emit pointsChanged();

                // We have deleted a point.
                return true;
            }
        }
    } break;

    case QEvent::MouseButtonRelease:
        _selectedPoint = emptyPoint;
         break;

    case QEvent::MouseMove:
    {
        // If there is a point selected with a specific index, move it.
        if( _selectedPoint == emptyPoint )
            return false;

        const auto& pos = ((QMouseEvent*)hoverEvent )->pos();
        const float y = std::max( std::min( 1.0f, 1.0f - pos.y() / h ), 0.0f );
        const float selectedX = _selectedPoint.x();

        // If it is the last point or the first point
        if( selectedX == _controlPoints.begin()->x())
        {
            // Update the control point only on y direction
            auto p = *(_controlPoints.begin());
            _controlPoints.erase( p );
            p.setY( y );
            _controlPoints.insert( p );
            emit pointsChanged();
            return true;
        }
        if( selectedX == _controlPoints.rbegin()->x())
        {
            // Update the control point only on y direction
            auto p = *(_controlPoints.rbegin());
            _controlPoints.erase( p );
            p.setY( y );
            _controlPoints.insert( p );
            emit pointsChanged();
            return true;
        }

        const float x = pos.x() / w;

        // If position reaches or passes the boundaries
        if( x <= _controlPoints.begin()->x()
            || x >= _controlPoints.rbegin()->x())
        {
            return true;
        }

        // If there is previously a control point in the position
        for( const auto& cp : _controlPoints )
        {
            if( cp.x() == x && _selectedPoint.x() != x )
                return true;
        }

        _controlPoints.erase( _selectedPoint );
        _selectedPoint = { x, y };
        _controlPoints.insert( _selectedPoint );

        // Update the system.
        emit pointsChanged();
        return true;

    } break;

    case QEvent::Paint: // Render the points on the widget.
    {
        QWidget* tfWidget = _colorMapWidget;
        _colorMapWidget = 0;

        QApplication::sendEvent( object, hoverEvent );
        _colorMapWidget = tfWidget;

        // Painting all the points on the widget.
        _paintPoints();
        return true;
    }
    default: break;
    }

    // No event is selected.
    return false;
}

void HoverPoints::_paintPoints()
{
    QPainter qPainter;
    qPainter.begin( _colorMapWidget );

    // Set anti-aliasing.
    qPainter.setRenderHint( QPainter::Antialiasing );

    QPolygonF polygon;

    const float w = _colorMapWidget->width();
    const float h = _colorMapWidget->height();

    for( const auto& point : _controlPoints )
        polygon << QPointF( point.x() * w, (1.0f - point.y()) * h );

    qPainter.setPen( connectionPen );
    qPainter.drawPolyline( polygon );

    // Set the style.
    qPainter.setPen( pointPen );
    qPainter.setBrush( pointBrush );

    // After drawing the lines between the points, draw the points' shapes.
    for( const auto& pnt: polygon )
        qPainter.drawEllipse( pnt, controlPointSize, controlPointSize );
}

QPolygonF HoverPoints::getControlPoints() const
{
    return QPolygonF::fromStdVector( { _controlPoints.begin(),
                                       _controlPoints.end() } );
}

void HoverPoints::setControlPoints( const QPolygonF& controlPoints )
{
    _controlPoints.clear();

    // clamp points to 0..1 range
    for( const auto& point : controlPoints )
    {
        _controlPoints.insert( { std::max( qreal(0),
                                           std::min( qreal(1), point.x( ))),
                                 std::max( qreal(0),
                                           std::min(qreal(1), point.y( )))
                               });
    }
}

}
