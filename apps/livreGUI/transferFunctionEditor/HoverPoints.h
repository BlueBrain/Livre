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

#ifndef _HoverPoints_h_
#define _HoverPoints_h_

#include <QtWidgets>

namespace livre
{

QT_FORWARD_DECLARE_CLASS( QBypassWidget )

/**
 * This class is used to create control points for color curves.
 */
class HoverPoints : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief The PointShape enum
     */
    enum PointShape
    {
        CIRCLE_POINT,
        SQUARE_SHAPE
    };

    /**
     * @brief The LockType enum
     */
    enum LockType
    {
        LOCK_TO_LEFT   = 0x01,
        LOCK_TO_RIGHT  = 0x02,
        LOCK_TO_TOP    = 0x04,
        LOCK_TO_BOTTOM = 0x08
    };

    /**
     * @brief The SortType enum
     */
    enum SortType
    {
        NO_SORT,
        X_SORT,
        Y_SORT
    };

    /**
     * @brief The ConnectionType enum
     */
    enum ConnectionType
    {
        NO_CONNECTION,
        LINE_CONNECTION,
        CURVE_CONNECTION
    };

    /**
     * The constructor of HoverPoints.
     * @param widget The parent widget.
     * @param shape The shape of the point (square or circular).
     */
    HoverPoints( QWidget *widget, PointShape shape );

    /**
     * Draw the control point.
     */
    void paintPoints();

    /**
     * Get the bounding rectangle of the point.
     */
    QRectF getBoundingRect() const;

    /**
     * Set the bounding rectangle of the point.
     * @param getBoundingRect The disired bounding rectangle.
     */
    void setBoundingRect( const QRectF& getBoundingRect );

    /**
     * Get the 2d positions of the control points.
     * @return QPolygonF containing 2d positons of the control points.
     */
    QPolygonF points() const;

    /**
     * Set the 2d positions of the control points.
     * @param listPoints QPolygonF containing 2d positons of the control points.
     */
    void setPoints( const QPolygonF& listPoints );

    /**
     * Get the size of the point.
     */
    QSizeF getPointSize() const;

    /**
     * Get the size of the point.
     * @param size The disired size of the point.
     */
    void setPointSize( const QSizeF& size );

    /**
     * Get the SORT_TYPE of the point.
     */
    SortType getSortType() const;

    /**
     * Set the SORT_TYPE of the point.
     * @param sortType The disired SORT_TYPE.
     */
    void setSortType( const SortType sortType );

    /**
     * Get the CONNECTION_TYPE of control points.
     */
    ConnectionType getConnectionType() const;

    /**
     * Set the CONNECTION_TYPE of control points.
     * @param connectionType The disired CONNECTION_TYPE.
     */
    void setConnectionType( const ConnectionType connectionType );

    /**
     * Set the QPen used to draw connections between control points.
     * @param pen The disired QPen.
     */
    void setConnectionPen( const QPen& pen );

    /**
     * Set the QPen used to draw the shape of control points.
     * @param pen The disired QPen.
     */
    void setShapePen( const QPen& pen );

    /**
     * Set the QBrush used to fill the shape of control points.
     * @param brush The disired QBrush.
     */
    void setShapeBrush( const QBrush& brush );

    /**
     * Set the LOCK_TYPE at current control points.
     * @param pos The index of the current control point.
     * @param lock The desired LOCK_TYPE.
     */
    void setPointLock( int pos, LockType lock );

    /**
     * Set "editable" boolean for the control points. If true, new control
     * points can be added.
     * @param editable The desired "editable" boolean.
     */
    void setEditable( const bool editable );

    /**
     * @brief Return the "editable" boolean for the control points. If true, new control
     * points can be added.
     */
    bool isEditable() const;

public:

    /**
     * @brief eventFilter
     * @param object
     * @param event
     * @return
     */
    bool eventFilter( QObject* object, QEvent* hoverEvent ) Q_DECL_OVERRIDE;

public slots:

    /**
     * setEnabled
     * @param enabled "Enabled" boolean.
     */
    void setEnabled( bool enabled );

    /**
     * setDisabled
     * @param disabled "Disabled" boolean.
     */
    void setDisabled( bool disabled ) { setEnabled(!disabled); }

signals:

    /**
     * pointsChanged signal.
     * @param points QPolygonF with changed points.
     */
    void pointsChanged( const QPolygonF& points );

public:

    /**
     * Fires pointsChanged() when one of the point changed.
     */
    void firePointChange();

private:
    QRectF _pointBoundingRectangle( int i ) const;

    void _movePoint( int i, const QPointF& newPos, bool emitChange = true );

    QWidget *_tfWidget;
    ConnectionType _connectionType;
    SortType _sortType;
    PointShape _pointShape;
    QPen _pointPen;
    QPen _connectionPen;
    QBrush _pointBrush;
    QSizeF _pointSize;
    int _currentPointIndex;
    QPolygonF _tfPoints;
    QRectF _bounds;
    QVector<uint> _locks;
    bool _editable;
    bool _enabled;
    QHash<int, int> _fingerPointMapping;
};

inline QRectF HoverPoints::_pointBoundingRectangle( const int i ) const
{
    const QPointF p = _tfPoints.at(i);
    const qreal w = _pointSize.width();
    const qreal h = _pointSize.height();
    const qreal x = p.x() - w / 2;
    const qreal y = p.y() - h / 2;
    return QRectF(x, y, w, h);
}

inline QRectF HoverPoints::getBoundingRect() const
{
    if ( _bounds.isEmpty())
        return _tfWidget->rect();
    else
        return _bounds;
}

}
#endif // _HoverPoints_h_
