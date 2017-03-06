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

#include "ControlPointsWidget.h"

#include <QMouseEvent>
#include <QQuickItem>

namespace livre
{
namespace
{
const float controlPointSize = 8.0f;
const QPointF emptyPoint(-1.0f, -1.0f);
}

ControlPointsWidget::ControlPointsWidget(QWidget* parent_,
                                         const Channel channel)
    : QQuickWidget(parent_)
    , _channel(channel)
    , _selectedPoint(emptyPoint)
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    setResizeMode(QQuickWidget::SizeRootObjectToView);
}

ControlPointsWidget::~ControlPointsWidget()
{
}

bool ControlPointsWidget::event(QEvent* event)
{
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if (mouseEvent)
    {
        if (_handleMouseEvent(mouseEvent))
        {
            emit colorsChanged();
            _updateControlPoints();
        }
    }
    return QQuickWidget::event(event);
}

uint32_t ControlPointsWidget::getColorAtPoint(const float xPosition) const
{
    const auto& controlPoints = getControlPoints();
    for (int i = 1; i < controlPoints.size(); ++i)
    {
        auto p1 = controlPoints.at(i - 1);
        auto p2 = controlPoints.at(i);

        if (p1.x() > xPosition || p2.x() < xPosition)
            continue;

        // transform normalized to onscreen absolute size for background pixel
        // lookup
        p1.setX(p1.x() * width());
        p1.setY((1.f - p1.y()) * height());
        p2.setX(p2.x() * width());
        p2.setY((1.f - p2.y()) * height());

        QLineF line(p1, p2);
        line.setLength(((xPosition * width()) - line.x1()) / line.dx() *
                       line.length());

        if (_channel == Channel::alpha)
        {
            const float alpha =
                std::min(1.f, float(line.y2()) / float(height() - 1));
            const uint32_t pixel =
                _background.pixel(qRound(qMin(line.x2(), qreal(width() - 1))),
                                  0);
            return (pixel & 0xffffffu) |
                   (qMin(unsigned((1.f - alpha) * 255), 255u) << 24u);
        }

        return _background.pixel(qRound(qMin(line.x2(), qreal(width() - 1))),
                                 qRound(qMin(line.y2(), qreal(height() - 1))));
    }
    return 0u;
}

UInt8s ControlPointsWidget::getCurve() const
{
    size_t currentHPointIndex = 0;

    const auto& controlPoints = getControlPoints();
    UInt8s curve;
    curve.reserve(COLORSAMPLES);
    for (size_t i = 0; i < COLORSAMPLES; ++i)
    {
        float normX = i / float(COLORSAMPLES - 1);
        if (controlPoints.at(currentHPointIndex + 1).x() < normX)
            currentHPointIndex++;

        const QLineF currentLine(controlPoints.at(currentHPointIndex),
                                 controlPoints.at(currentHPointIndex + 1));
        const float slope = currentLine.dy() / currentLine.dx();
        const float normY =
            (normX - currentLine.p1().x()) * slope + currentLine.p1().y();

        const uint8_t currentCurveValue = 255u * normY;
        curve.push_back(currentCurveValue);
    }
    return curve;
}

bool ControlPointsWidget::_handleMouseEvent(QMouseEvent* mouseEvent)
{
    const float w = width();
    const float h = height();

    // Detect the event type.
    switch (mouseEvent->type())
    {
    case QEvent::MouseButtonPress:
    {
        QPointF clickPosition = mouseEvent->pos();
        QPointF selectedPoint = emptyPoint;
        for (const auto& cp : _controlPoints)
        {
            // Select the shape of the bounding rectangle of the volume
            // whether it is circle of rectangle.
            QPainterPath touchSurface;
            const QPointF point(cp.x() * w, (1.0f - cp.y()) * h);
            touchSurface.addEllipse(point, controlPointSize, controlPointSize);

            // If the mouse event was applied in this boundary of the point,
            // set the index to that of the selected point.
            if (touchSurface.contains(clickPosition))
            {
                selectedPoint = {cp.x(), cp.y()};
                break;
            }
        }

        // If the Qt::LeftButton is clicked where there are no points,
        // insert a new point, and if the Qt::LeftButton is clicked where
        // a point already exists, then move the HoverPoint.
        if (mouseEvent->button() == Qt::LeftButton)
        {
            // If there is no point where the mouse is clicked, then create
            // a new point.
            if (selectedPoint == emptyPoint)
            {
                // Insert a new point at this position.
                _selectedPoint = QPointF((float)clickPosition.x() / w,
                                         1.0f - (float)clickPosition.y() / h);

                _controlPoints.insert(_selectedPoint);

                return true;
            }

            // If there is a specific point that is clicked, get it.
            _selectedPoint = selectedPoint;

            return false;
        }

        // If the Qt::RightButton is selcted where there is a point, then
        // delete this point and update the system.
        if (mouseEvent->button() == Qt::RightButton)
        {
            // If there is a specified point that is selected based on
            // the index and the widget is editible.
            if (selectedPoint != emptyPoint)
            {
                const float selectedX = selectedPoint.x();

                // If it is the last point or the first point, do not remove
                if (selectedX == _controlPoints.begin()->x() ||
                    selectedX == _controlPoints.rbegin()->x())
                {
                    return false;
                }

                _controlPoints.erase(selectedPoint);

                return true;
            }
        }
    }
    break;

    case QEvent::MouseButtonRelease:
        _selectedPoint = emptyPoint;
        break;

    case QEvent::MouseMove:
    {
        // If there is a point selected with a specific index, move it.
        if (_selectedPoint == emptyPoint)
            return false;

        const auto& pos = mouseEvent->pos();
        const float y = std::max(std::min(1.0f, 1.0f - pos.y() / h), 0.0f);
        const float selectedX = _selectedPoint.x();

        // If it is the last point or the first point
        if (selectedX == _controlPoints.begin()->x())
        {
            // Update the control point only on y direction
            auto p = *(_controlPoints.begin());
            _controlPoints.erase(p);
            p.setY(y);
            _controlPoints.insert(p);
            return true;
        }
        if (selectedX == _controlPoints.rbegin()->x())
        {
            // Update the control point only on y direction
            auto p = *(_controlPoints.rbegin());
            _controlPoints.erase(p);
            p.setY(y);
            _controlPoints.insert(p);
            return true;
        }

        const float x = pos.x() / w;

        // If position reaches or passes the boundaries
        if (x <= _controlPoints.begin()->x() ||
            x >= _controlPoints.rbegin()->x())
        {
            return false;
        }

        // If there is previously a control point in the position
        for (const auto& cp : _controlPoints)
        {
            if (cp.x() == x && _selectedPoint.x() != x)
                return false;
        }

        _controlPoints.erase(_selectedPoint);
        _selectedPoint = {x, y};
        _controlPoints.insert(_selectedPoint);

        return true;
    }
    break;

    default:
        break;
    }

    return false;
}

QPolygonF ControlPointsWidget::getControlPoints() const
{
    return QPolygonF::fromStdVector(
        {_controlPoints.begin(), _controlPoints.end()});
}

void ControlPointsWidget::setControlPoints(const QPolygonF& controlPoints)
{
    _controlPoints.clear();

    // clamp points to 0..1 range
    for (const auto& point : controlPoints)
    {
        _controlPoints.insert(
            {std::max(qreal(0), std::min(qreal(1), point.x())),
             std::max(qreal(0), std::min(qreal(1), point.y()))});
    }

    _updateControlPoints();
}

void ControlPointsWidget::_updateControlPoints()
{
    QList<QVariant> controlPointList;
    for (const auto& point : getControlPoints())
        controlPointList.push_back(QVariant::fromValue(point));
    rootObject()->setProperty("controlpoints", QVariant(controlPointList));
}
}
