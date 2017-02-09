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

#ifndef _HoverPoints_h_
#define _HoverPoints_h_

#include <QWidget>
#include <livreGUI/types.h>

namespace livre
{

/**
 * This class is used to create control points for color curves.
 */
class HoverPoints : public QObject
{
    Q_OBJECT
public:
    HoverPoints( QWidget *widget );

    /**
     * Get the 2d positions of the control points.
     * @return QPolygonF containing 2d positons of the control points.
     */
    QPolygonF getControlPoints() const;

    /**
     * Set the 2d positions of the control points.
     * @param controlPoints QPolygonF containing 2d positons of the control points.
     */
    void setControlPoints( const QPolygonF& controlPoints );

public:
    bool eventFilter( QObject* object, QEvent* hoverEvent ) final;

signals:
    void pointsChanged();

private:
    void _paintPoints();

    QWidget* _colorMapWidget;
    ControlPoints _controlPoints = ControlPoints( compareControlPoints );
    QPointF _selectedPoint;
};

}
#endif // _HoverPoints_h_
