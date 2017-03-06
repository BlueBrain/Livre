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

#pragma once

#include <livreGUI/types.h>

#include <QImage>
#include <QMouseEvent>
#include <QQuickWidget>

namespace livre
{
/** A base widget providing a curve to change the associated channel's value. */
class ControlPointsWidget : public QQuickWidget
{
    Q_OBJECT

public:
    enum class Channel
    {
        red,
        green,
        blue,
        alpha
    };

    ControlPointsWidget(QWidget* parent, Channel channel);
    virtual ~ControlPointsWidget();

    Channel getChannel() const { return _channel; }
    QPolygonF getControlPoints() const;
    void setControlPoints(const QPolygonF& controlPoints);
    uint32_t getColorAtPoint(float xPosition) const;
    UInt8s getCurve() const;

signals:
    void colorsChanged();

protected:
    QImage _background;

private:
    bool event(QEvent* event) final;

    bool _handleMouseEvent(QMouseEvent* mouseEvent);
    void _updateControlPoints();

    Channel _channel;
    ControlPoints _controlPoints = ControlPoints(compareControlPoints);
    QPointF _selectedPoint;
};
}
