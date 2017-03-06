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

#include <QWidget>

#include <lexis/render/Histogram.h>

namespace Ui
{
class RangeWidget;
}

namespace livre
{
/**
 * A widget showing the histogram of the entire data range and providing a range
 * selector to 'zoom' in the data range.
 */
class RangeWidget : public QWidget
{
    Q_OBJECT

public:
    RangeWidget(QWidget* parent);
    ~RangeWidget();

    Q_PROPERTY(
        qreal rangeX READ getRangeX WRITE setRangeX NOTIFY rangeXChanged);
    Q_PROPERTY(
        qreal rangeY READ getRangeY WRITE setRangeY NOTIFY rangeYChanged);

    void setHistogram(const lexis::render::Histogram& histogram,
                      bool isLogScale);

    qreal getRangeX() const;
    qreal getRangeY() const;

    vmml::Vector2d fromNormalizedRange() const;

signals:
    void rangeChanged(vmml::Vector2f range);
    void histIndexChanged(size_t index, double ratio);
    void rangeXChanged(qreal);
    void rangeYChanged(qreal);

private slots:
    void setRangeX(qreal);
    void setRangeY(qreal);

private:
    bool eventFilter(QObject* watched, QEvent* event) final;
    QSize sizeHint() const final { return {255, 255}; }
    Ui::RangeWidget* _ui;
    lexis::render::Histogram _histogram;
    bool _isLogScale{true};
    qreal _rangeX{0};
    qreal _rangeY{1};

    qreal _convertFromNormalizedRange(qreal value) const;
    qreal _convertToNormalizedRange(qreal value) const;
};
}
