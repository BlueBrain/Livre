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

#pragma once

#include <livreGUI/transferFunctionEditor/ControlPointsWidget.h>

#include <lexis/render/Histogram.h>

#include <QLinearGradient>

#include <vmmlib/vector.hpp>

namespace livre
{
/**
 * A widget showing the color gradient, providing a curve to change the alpha
 * values and showing the histogram of the selected data range.
 */
class AlphaWidget : public ControlPointsWidget
{
    Q_OBJECT

public:
    AlphaWidget(QWidget* parent);

    void setGradientStops(const QGradientStops& stops);
    void setHistogram(const lexis::render::Histogram& histogram,
                      bool isLogScale);
    void setRange(const vmml::Vector2f& range);

signals:
    void histIndexChanged(size_t index, double ratio);

private:
    QSize sizeHint() const final { return {255, 255}; }
    void mouseMoveEvent(QMouseEvent* event) final;
    void leaveEvent(QEvent* event) final;
    void resizeEvent(QResizeEvent*) final;

    lexis::render::Histogram _histogram;
    bool _isLogScale{true};
    QLinearGradient _gradient{0, 0, 0, 0};
    vmml::Vector2f _range{0, 1};
};
}
