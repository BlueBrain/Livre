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

#include "RangeWidget.h"
#include "Utils.h"

#include <livreGUI/ui_RangeWidget.h>

#include <QMouseEvent>
#include <QQmlContext>
#include <QQuickItem>

#include <vmmlib/vector.hpp>

namespace livre
{
RangeWidget::RangeWidget(QWidget* parent_)
    : QWidget(parent_)
    , _ui(new Ui::RangeWidget)
{
    _ui->setupUi(this);

    _ui->background->installEventFilter(this);
    _ui->background->rootContext()->setContextProperty("model", this);

    connect(_ui->fitToDataButton, &QPushButton::clicked, [this] {
        setRangeX(0);
        setRangeY(1);
    });

    connect(_ui->minDataRange, static_cast<void (QDoubleSpinBox::*)(double)>(
                                   &QDoubleSpinBox::valueChanged),
            [this](const double value) {
                _ui->maxDataRange->setMinimum(value);
                if (!_histogram.isEmpty())
                    setRangeX(_convertToNormalizedRange(value));
            });
    connect(this, &RangeWidget::rangeXChanged, [this](qreal value) {
        if (_histogram.isEmpty())
            return;
        value = _convertFromNormalizedRange(value);
        _ui->minDataRange->setMinimum(
            std::min(_ui->minDataRange->minimum(), double(value)));
        _ui->minDataRange->setValue(value);
    });

    auto leftRange =
        _ui->background->rootObject()->findChild<QObject*>("leftrange");
    connect(leftRange, SIGNAL(viewposChanged(qreal)), this,
            SLOT(setRangeX(qreal)));

    connect(_ui->maxDataRange, static_cast<void (QDoubleSpinBox::*)(double)>(
                                   &QDoubleSpinBox::valueChanged),
            [this](const double value) {
                _ui->minDataRange->setMaximum(value);
                if (!_histogram.isEmpty())
                    setRangeY(_convertToNormalizedRange(value));
            });
    connect(this, &RangeWidget::rangeYChanged, [this](qreal value) {
        if (_histogram.isEmpty())
            return;
        value = _convertFromNormalizedRange(value);
        _ui->maxDataRange->setMaximum(
            std::max(_ui->maxDataRange->maximum(), double(value)));
        _ui->maxDataRange->setValue(value);
    });

    auto rightRange =
        _ui->background->rootObject()->findChild<QObject*>("rightrange");
    connect(rightRange, SIGNAL(viewposChanged(qreal)), this,
            SLOT(setRangeY(qreal)));
}

RangeWidget::~RangeWidget()
{
    delete _ui;
}

void RangeWidget::setHistogram(const lexis::render::Histogram& histogram,
                               const bool isLogScale)
{
    if (histogram == _histogram && isLogScale == _isLogScale)
        return;

    const bool haveHistogram = !histogram.isEmpty();
    _ui->minDataRange->setEnabled(haveHistogram);
    _ui->maxDataRange->setEnabled(haveHistogram);
    _ui->fitToDataButton->setEnabled(haveHistogram);

    _histogram = histogram;
    _isLogScale = isLogScale;

    emit rangeXChanged(_rangeX);
    emit rangeYChanged(_rangeY);

    _ui->background->rootObject()->setProperty("histogram",
                                               sampleHistogram(_histogram,
                                                               _isLogScale));
}

bool RangeWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == _ui->background && event->type() == QEvent::MouseMove)
    {
        if (_histogram.isEmpty())
            return true;

        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        const size_t index = (float)_histogram.getBins().size() *
                             (float)mouseEvent->pos().x() / (float)width();
        emit histIndexChanged(index, _histogram.getRatio(index));
    }
    return false;
}

void RangeWidget::setRangeX(const qreal rangeX)
{
    if (qFuzzyCompare(_rangeX, rangeX) || !std::isfinite(rangeX))
        return;

    _rangeX = rangeX;
    emit rangeXChanged(_rangeX);
    emit rangeChanged({float(_rangeX), float(_rangeY)});
}

qreal RangeWidget::getRangeX() const
{
    return _rangeX;
}

void RangeWidget::setRangeY(const qreal rangeY)
{
    if (qFuzzyCompare(_rangeY, rangeY) || !std::isfinite(rangeY))
        return;

    _rangeY = rangeY;
    emit rangeYChanged(_rangeY);
    emit rangeChanged({float(_rangeX), float(_rangeY)});
}

qreal RangeWidget::getRangeY() const
{
    return _rangeY;
}

vmml::Vector2d RangeWidget::fromNormalizedRange() const
{
    return {_convertFromNormalizedRange(_rangeX),
            _convertFromNormalizedRange(_rangeY)};
}

qreal RangeWidget::_convertFromNormalizedRange(const qreal value) const
{
    const double range = _histogram.getMax() - _histogram.getMin();
    return value * range + _histogram.getMin();
}

qreal RangeWidget::_convertToNormalizedRange(const qreal value) const
{
    const double range = _histogram.getMax() - _histogram.getMin();
    return (value - _histogram.getMin()) / range;
}
}
