/* Copyright (c) 2017, EPFL/Blue Brain Project
 *                     Daniel.Nachbaur@epfl.ch
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

#include "AlphaWidget.h"
#include "Utils.h"

#include <QMouseEvent>
#include <QPainter>
#include <QQuickItem>
#include <QVector4D>

namespace
{
const QUrl QML_ROOT_COMPONENT("qrc:/qml/AlphaWidget.qml");
}

namespace livre
{
AlphaWidget::AlphaWidget(QWidget* parent_)
    : ControlPointsWidget(parent_, Channel::alpha)
{
    setSource(QML_ROOT_COMPONENT);
}

void AlphaWidget::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    size_t index = (float)_histogram.getBins().size() *
                   (float)mouseEvent->pos().x() / (float)width();
    index *= (_range[1] - _range[0]);
    index += _histogram.getBins().size() * _range[0];
    emit histIndexChanged(index, _histogram.getRatio(index));
}

void AlphaWidget::leaveEvent(QEvent*)
{
    emit histIndexChanged(-1u, 0.0f);
}

void AlphaWidget::resizeEvent(QResizeEvent* ev)
{
    ControlPointsWidget::resizeEvent(ev);

    if (_background.isNull() || _background.size() != ev->size())
    {
        _background = QImage(size(), QImage::Format_ARGB32_Premultiplied);

        _gradient.setFinalStop(width(), 0);
        _background.fill(0);
        QPainter painter(&_background);
        painter.fillRect(_background.rect(), _gradient);
    }
}

void AlphaWidget::setGradientStops(const QGradientStops& stops)
{
    // the gradient here is needed for the color lookup in getColorAtPoint().
    // The visual appearance is performed by QML.
    _gradient = QLinearGradient(0.0f, 0.0f, width(), 0.0f);
    for (int i = 0; i < stops.size(); ++i)
        _gradient.setColorAt(stops.at(i).first, stops.at(i).second);

    // convert the gradient stops to something that QML understands
    QList<QVariant> positions;
    QList<QVariant> colors;
    for (const auto& stop : stops)
    {
        positions.push_back(stop.first);
        const auto& color = stop.second;
        colors.push_back(QVector4D(color.redF(), color.greenF(), color.blueF(),
                                   color.alphaF()));
    }

    rootObject()->setProperty("positions", positions);
    rootObject()->setProperty("colors", colors);
}

void AlphaWidget::setHistogram(const lexis::render::Histogram& histogram,
                               const bool isLogScale)
{
    if (_histogram == histogram && isLogScale == _isLogScale)
        return;

    _histogram = histogram;
    _isLogScale = isLogScale;
    rootObject()->setProperty("histogram",
                              sampleHistogram(_histogram, _isLogScale, _range));
}

void AlphaWidget::setRange(const vmml::Vector2f& range)
{
    if (_range == range)
        return;

    _range = range;
    rootObject()->setProperty("histogram",
                              sampleHistogram(_histogram, _isLogScale, _range));
}
}
