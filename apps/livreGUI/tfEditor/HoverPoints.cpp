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

#include <livreGUI/tfEditor/HoverPoints.h>

namespace livre
{

HoverPoints::HoverPoints( QWidget* widget, PointShape shape )
    : QObject( widget )
    , _tfWidget( widget )
    , _connectionType( CURVE_CONNECTION )
    , _sortType( NO_SORT )
    , _pointShape( shape )
    , _pointPen( QPen( QColor( 255, 255, 255, 191 ), 1 ))
    , _connectionPen( QPen( QColor( 255, 255, 255, 127 ), 2 ))
    , _pointBrush( QBrush( QColor( 191, 191, 191, 127 )))
    , _pointSize( QSize( 22, 22 ))
    , _currentPointIndex( -1 )
    , _editable( true )
    , _enabled( true )
{
    widget->installEventFilter( this );
    widget->setAttribute( Qt::WA_AcceptTouchEvents );

    connect( this, SIGNAL( pointsChanged( QPolygonF )), _tfWidget, SLOT( update()));
}

void HoverPoints::setEnabled( bool enabled )
{
    if( _enabled != enabled )
    {
        _enabled = enabled;
        _tfWidget->update();
    }
}

bool HoverPoints::eventFilter( QObject* object, QEvent* hoverEvent )
{
    // If the selected object is the transfer object and the widget is enabled.
    if( object == _tfWidget && _enabled )
    {
        // Detect the event type.
        switch ( hoverEvent->type())
        {
        case QEvent::MouseButtonPress:
        {
            if( !_fingerPointMapping.isEmpty())
                return true;

            QMouseEvent* mouseEvent = (QMouseEvent*)hoverEvent;
            QPointF clickPosition = mouseEvent->pos();
            int index = -1;
            for( int32_t i = 0; i < _tfPoints.size(); ++i )
            {
                // Select the shape of the bounding rectangle of the volume
                // whether it is circle of rectangle.
                QPainterPath touchSurface;
                if( _pointShape == CIRCLE_POINT )
                    touchSurface.addEllipse( _pointBoundingRectangle( i ));
                else
                    touchSurface.addRect( _pointBoundingRectangle( i ));

                // If the mouse event was applied in this boundary of the point,
                // set the index to that of the selected point.
                if (touchSurface.contains( clickPosition ))
                {
                    index = i;
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
                if( index == -1 )
                {
                    // If the widget (point) is not editible, return.
                    if ( !_editable ) return false;

                    // TODO: Insert sort for x or y
                    int position = 0;
                    if( _sortType == X_SORT )
                    {
                        for( int32_t i = 0; i < _tfPoints.size(); ++i )
                        {
                            if( _tfPoints.at(i).x() > clickPosition.x())
                            {
                                position = i;
                                break;
                            }
                        }
                    }
                    else if( _sortType == Y_SORT )
                    {
                        for ( int32_t i = 0; i < _tfPoints.size(); ++i )
                        {
                            if( _tfPoints.at(i).y() > clickPosition.y())
                            {
                                position = i;
                                break;
                            }
                        }
                    }

                    // Insert a new point at this position.
                    _tfPoints.insert( position, clickPosition );
                    _locks.insert( position, 0 );
                    _currentPointIndex = position;

                    // Update the system.
                    firePointChange();
                }
                else // If there is a specific point that is clicked, get it.
                {
                    _currentPointIndex = index;
                }

                // We have created or selected a point.
                return true;

            }
            // If the Qt::RightButton is selcted where there is a point, then
            // delete this point and update the system.
            else if( mouseEvent->button() == Qt::RightButton )
            {
                // If there is a specified point that is selected based on
                // the index and the widget is editible.
                if( index >= 0 && _editable )
                {
                    if( _locks[index] == 0 )
                    {
                        // Remove the point from the list.
                        _locks.remove( index );
                        _tfPoints.remove( index );
                    }

                    // Update the system.
                    firePointChange();

                    // We have deleted a point.
                    return true;
                }
            }
        }   break;

        case QEvent::MouseButtonRelease:
        {
            if( !_fingerPointMapping.isEmpty())
                return true;
            _currentPointIndex = -1;
        }   break;

        case QEvent::MouseMove:
        {
            // If there is no point selected, do nothing.
            if( !_fingerPointMapping.isEmpty())
                return true;

            // If there is a point selected with a specific index, move it.
            if( _currentPointIndex >= 0 )
                _movePoint( _currentPointIndex, ((QMouseEvent*)hoverEvent )->pos());
        }   break;


        case QEvent::TouchBegin:  /// Events for the rendeirng widget.
        case QEvent::TouchUpdate: /// Events for the rendeirng widget.
        {
            const QTouchEvent* const touchEvent =
                    static_cast<const QTouchEvent*>( hoverEvent );
            const QList<QTouchEvent::TouchPoint> touchPoints =
                    touchEvent->touchPoints();
            const qreal pointSize =
                    qMax( _pointSize.width(), _pointSize.height() );

            foreach ( const QTouchEvent::TouchPoint &touchPoint, touchPoints )
            {
                const int touchId = touchPoint.id();
                switch ( touchPoint.state())
                {
                case Qt::TouchPointPressed:
                {
                    // Find the point and then move it
                    QSet<int> activePoints =
                            QSet<int>::fromList( _fingerPointMapping.values());

                    int activePoint = -1;
                    qreal distance = -1;
                    const int pointsCount = _tfPoints.size();
                    const int activePointCount = activePoints.size();

                    // You are allowed to only have two points on the rendering
                    // widget.
                    if( pointsCount == 2 && activePointCount == 1 )
                        activePoint = activePoints.contains(0) ? 1 : 0;
                    else
                    {
                        for( int32_t i = 0; i < pointsCount; ++i )
                        {
                            if( activePoints.contains( i ))
                                continue;

                            qreal d = QLineF( touchPoint.pos(), _tfPoints.at(i)).length();
                            if(( distance < 0 && d < 12 * pointSize) || d < distance )
                            {
                                distance = d;
                                activePoint = i;
                            }
                        }
                    }

                    if( activePoint != -1 )
                    {
                        _fingerPointMapping.insert( touchPoint.id(), activePoint );
                        _movePoint( activePoint, touchPoint.pos());
                    }
                }
                break;

                case Qt::TouchPointReleased: /// Events for the rendeirng widget.
                {
                    // Move the point and release
                    QHash<int,int>::iterator it = _fingerPointMapping.find( touchId );
                    _movePoint( it.value(), touchPoint.pos());
                    _fingerPointMapping.erase( it );
                }
                break;

                case Qt::TouchPointMoved: /// Events for the rendeirng widget.
                {
                    // Move the point
                    const int pointIdx = _fingerPointMapping.value( touchId, -1 );
                    if( pointIdx >= 0 )
                        _movePoint(pointIdx, touchPoint.pos());
                }
                break;

                default:
                    break;
                }
            }
            if( _fingerPointMapping.isEmpty())
            {
                hoverEvent->ignore();
                return false;
            }
            else
            {
                return true;
            }
        }
            break;
        case QEvent::TouchEnd:
            if( _fingerPointMapping.isEmpty())
            {
                hoverEvent->ignore();
                return false;
            }
            return true;
            break;

        case QEvent::Resize: // Resize the points.
        {
            QResizeEvent* resizeEvent = (QResizeEvent *) hoverEvent;
            const int oldPointWidth = resizeEvent->oldSize().width();
            const int oldPointHeight = resizeEvent->oldSize().height();
            if( oldPointWidth == 0 || oldPointHeight == 0 )
                break;

            const int newPointWidth = resizeEvent->size().width();
            const int newPointHeight = resizeEvent->size().height();
            const qreal scaleX = newPointWidth / qreal( oldPointWidth );
            const qreal scaleY = newPointHeight / qreal( oldPointHeight );

            // Update the size of all the points in the transfer function widget.
            for( int32_t i = 0; i < _tfPoints.size(); ++i )
            {
                QPointF p = _tfPoints[i];
                _movePoint( i, QPointF(p.x() * scaleX, p.y() * scaleY), false );
            }

            // Update the system.
            firePointChange();
            break;
        }

        case QEvent::Paint: // Render the points on the widget.
        {
            QWidget* tfWidget = _tfWidget;
            _tfWidget = 0;

            QApplication::sendEvent( object, hoverEvent );
            _tfWidget = tfWidget;

            // Paing all the points on the widget.
            paintPoints();
            return true;
        }
        default: break;
        }
    }

    // No event is selected.
    return false;
}

void HoverPoints::paintPoints()
{
    QPainter qPainter;
    qPainter.begin( _tfWidget );

    // Set anti-aliasing.
    qPainter.setRenderHint( QPainter::Antialiasing );

    // Make sure that styles and connection type are defined.
    if( _connectionPen.style() != Qt::NoPen && _connectionType != NO_CONNECTION )
    {
        qPainter.setPen( _connectionPen );

        // Connect the points using smoothed curves.
        if( _connectionType == CURVE_CONNECTION )
        {
            QPainterPath path;
            path.moveTo( _tfPoints.at( 0 ));
            for( int32_t i = 1; i < _tfPoints.size(); ++i )
            {
                const QPointF& p1 = _tfPoints.at( i - 1 );
                const QPointF& p2 = _tfPoints.at( i );
                const qreal distance = p2.x() - p1.x();

                path.cubicTo( p1.x() + distance / 2, p1.y(),
                              p1.x() + distance / 2, p2.y(),
                              p2.x(), p2.y());
            }

            qPainter.drawPath( path );
        }
        // Connect the points using piece-wise lines.
        else
            qPainter.drawPolyline( _tfPoints );
    }

    // Set the style.
    qPainter.setPen( _pointPen );
    qPainter.setBrush( _pointBrush );

    // After drawinf the lines between the points, draw the points' shapes.
    for( int32_t i = 0; i < _tfPoints.size(); ++i )
    {
        const QRectF& bounds = _pointBoundingRectangle( i );
        if( _pointShape == CIRCLE_POINT )
            qPainter.drawEllipse( bounds );
        else
            qPainter.drawRect( bounds );
    }
}

static QPointF getBoundPoint( const QPointF& point, const QRectF& bounds,
                              const int lock )
{
    QPointF selectedPoint = point;

    const qreal left = bounds.left();
    const qreal right = bounds.right();
    const qreal top = bounds.top();
    const qreal bottom = bounds.bottom();

    if( selectedPoint.x() < left || ( lock & HoverPoints::LOCK_TO_LEFT ))
        selectedPoint.setX( left );
    else if( selectedPoint.x() > right || ( lock & HoverPoints::LOCK_TO_RIGHT ))
        selectedPoint.setX( right );

    if( selectedPoint.y() < top || ( lock & HoverPoints::LOCK_TO_TOP ))
        selectedPoint.setY( top );
    else if( selectedPoint.y() > bottom || ( lock & HoverPoints::LOCK_TO_BOTTOM ))
        selectedPoint.setY( bottom );

    return selectedPoint;
}

void HoverPoints::setPoints( const QPolygonF& listPoints )
{
    // If the given list of points is not equal to the current list,
    // clear all the points.
    if( listPoints.size() != _tfPoints.size())
        _fingerPointMapping.clear();
    _tfPoints.clear();

    // Update the current _tfPoints_ list to the given list of points.
    for( int32_t i = 0; i < listPoints.size(); ++i )
        _tfPoints << getBoundPoint( listPoints.at(i), getBoundingRect(), 0 );

    // Update the new list of points before drawinf them.
    _locks.clear();
    if( _tfPoints.size() > 0 )
    {
        _locks.resize( _tfPoints.size());
        _locks.fill(0);
    }
}

void HoverPoints::_movePoint( const int index, const QPointF& point,
                              const bool emitUpdate )
{
    // Get the point bounding object to move it.
    _tfPoints[index] = getBoundPoint( point, getBoundingRect(), _locks.at( index ));

    if( emitUpdate )
        firePointChange();
}

inline static bool xLessThan( const QPointF& p1, const QPointF& p2 )
{
    return p1.x() < p2.x();
}

inline static bool yLessThan( const QPointF& p1, const QPointF& p2 )
{
    return p1.y() < p2.y();
}

void HoverPoints::firePointChange()
{
    if( _sortType != NO_SORT )
    {
        QPointF oldCurrent;
        if( _currentPointIndex != -1 )
            oldCurrent = _tfPoints[_currentPointIndex];

        if( _sortType == X_SORT )
            std::sort( _tfPoints.begin(), _tfPoints.end(), xLessThan );
        else if( _sortType == Y_SORT )
            std::sort( _tfPoints.begin(), _tfPoints.end(), yLessThan );

        // Compensate for changed order ...
        if( _currentPointIndex != -1 )
        {
            for( int32_t i = 0; i < _tfPoints.size(); ++i )
            {
                if( _tfPoints[i] == oldCurrent )
                {
                    _currentPointIndex = i;
                    break;
                }
            }
        }
    }

    // Emit the signal to activate the change.
    emit pointsChanged( _tfPoints );
}

void HoverPoints::setBoundingRect( const QRectF& boundingRect )
{
    _bounds = boundingRect;
}

QPolygonF HoverPoints::points() const
{
    return _tfPoints;
}

QSizeF HoverPoints::getPointSize() const
{
    return _pointSize;
}

void HoverPoints::setPointSize( const QSizeF& size )
{
    _pointSize = size;
}

HoverPoints::SortType HoverPoints::getSortType() const
{
    return _sortType;
}

void HoverPoints::setSortType( const HoverPoints::SortType sortType )
{
    _sortType = sortType;
}

HoverPoints::ConnectionType HoverPoints::getConnectionType() const
{
    return _connectionType;
}

void HoverPoints::setConnectionType( HoverPoints::ConnectionType connectionType )
{
    _connectionType = connectionType;
}

void HoverPoints::setConnectionPen( const QPen& pen )
{
    _connectionPen = pen;
}

void HoverPoints::setShapePen( const QPen& pen )
{
    _pointPen = pen;
}

void HoverPoints::setShapeBrush( const QBrush& brush )
{
    _pointBrush = brush;
}

void HoverPoints::setPointLock( int pos, HoverPoints::LockType lock )
{
    _locks[pos] = lock;
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
