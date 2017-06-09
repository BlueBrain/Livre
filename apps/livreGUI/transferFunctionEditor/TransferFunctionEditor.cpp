/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Marwan Abdellah <marwan.abdellah@epfl.ch>
 *                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#include "TransferFunctionEditor.h"
#include "AlphaWidget.h"
#include "ColorWidget.h"
#include "RangeWidget.h"

#include <livreGUI/Controller.h>
#include <livreGUI/ui_TransferFunctionEditor.h>

#include <lexis/render/materialLUT.h>

#include <vmmlib/vector.hpp>

#include <QFileDialog>

#include <cmath>
#include <fstream>

namespace livre
{
namespace
{
const ColorWidget::Channel channels[] = {ColorWidget::Channel::red,
                                         ColorWidget::Channel::green,
                                         ColorWidget::Channel::blue,
                                         ColorWidget::Channel::alpha};
}

TransferFunctionEditor::TransferFunctionEditor(Controller& controller,
                                               QWidget* parent_)
    : QWidget(parent_)
    , _controller(controller)
    , _ui(new Ui::TransferFunctionEditor)
{
    _ui->setupUi(this);

    _rangeWidget = new RangeWidget(this);
    connect(_rangeWidget, SIGNAL(histIndexChanged(size_t, double)), this,
            SLOT(_onHistIndexChanged(size_t, double)));
    connect(_rangeWidget, SIGNAL(rangeChanged(vmml::Vector2f)), this,
            SLOT(_onRangeChanged(vmml::Vector2f)));
    _ui->widgetsLayout->addWidget(_rangeWidget);

    _alphaWidget = new AlphaWidget(this);
    connect(_alphaWidget, SIGNAL(colorsChanged()), this,
            SIGNAL(transferFunctionChanged()));
    connect(_alphaWidget, SIGNAL(histIndexChanged(size_t, double)), this,
            SLOT(_onHistIndexChanged(size_t, double)));
    _ui->widgetsLayout->addWidget(_alphaWidget);
    _controlPointsWidgets[3] = _alphaWidget;

    for (const auto& channel : channels)
    {
        if (channel == ColorWidget::Channel::alpha)
            break;

        auto widget = new ColorWidget(this, channel);
        _ui->widgetsLayout->addWidget(widget);
        _controlPointsWidgets[(size_t)channel] = widget;
        connect(widget, SIGNAL(colorsChanged()), this,
                SIGNAL(transferFunctionChanged()));
    }

    connect(_ui->histogramScaleCheckBox, SIGNAL(clicked(bool)), this,
            SLOT(_onHistogramChanged(bool)));

    connect(_ui->resetButton, SIGNAL(clicked()), this, SLOT(_setDefault()));
    connect(_ui->loadButton, SIGNAL(clicked()), this, SLOT(_load()));
    connect(_ui->saveButton, SIGNAL(clicked()), this, SLOT(_save()));

    connect(this, &TransferFunctionEditor::transferFunctionChanged, this,
            &TransferFunctionEditor::_onTransferFunctionChanged,
            Qt::QueuedConnection);

    connect(this, &TransferFunctionEditor::histogramChanged, this,
            &TransferFunctionEditor::_onHistogramChanged, Qt::QueuedConnection);

    _controller.subscribe(_histogram);
    _histogram.registerDeserializedCallback([&] {
        emit histogramChanged(_ui->histogramScaleCheckBox->checkState() ==
                              Qt::Checked);
    });

    _setDefault();
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    _controller.unsubscribe(_histogram);
    delete _ui;
}

void TransferFunctionEditor::_setGradientStops()
{
    ControlPoints allControlPoints(compareControlPoints);
    for (auto& widget : _controlPointsWidgets)
    {
        const auto& controlPoints = widget->getControlPoints();
        allControlPoints.insert(controlPoints.begin(), controlPoints.end());
    }

    QGradientStops stops;
    constexpr size_t nChannels = 4;
    for (const auto& point : allControlPoints)
    {
        int colors[nChannels] = {0};
        for (size_t j = 0; j < nChannels; ++j)
            colors[j] = _controlPointsWidgets[j]->getColorAtPoint(point.x());

        QColor color((0x00ff0000 & colors[0]) >> 16,  // R (16)
                     (0x0000ff00 & colors[1]) >> 8,   // G (8)
                     (0x000000ff & colors[2]),        // B (1)
                     (0xff000000 & colors[3]) >> 24); // A (24)

        stops << QGradientStop(point.x(), color);
    }

    _alphaWidget->setGradientStops(stops);
}

void TransferFunctionEditor::_setDefault()
{
    for (auto& widget : _controlPointsWidgets)
    {
        QPolygonF points;
        switch (widget->getChannel())
        {
        case ColorWidget::Channel::red:
        case ColorWidget::Channel::green:
            points << QPointF(0.0, 0.0);
            points << QPointF(0.4, 0.0);
            points << QPointF(0.6, 1.0);
            points << QPointF(1.0, 1.0);
            break;
        case ColorWidget::Channel::blue:
            points << QPointF(0.0, 0.0);
            points << QPointF(0.2, 1.0);
            points << QPointF(0.6, 1.0);
            points << QPointF(0.8, 0.0);
            points << QPointF(1.0, 0.0);
            break;
        case ColorWidget::Channel::alpha:
            points << QPointF(0.0, 0.0);
            points << QPointF(0.1, 0.2);
            points << QPointF(1.0, 0.8);
            break;
        }
        widget->setControlPoints(points);
    }

    emit transferFunctionChanged();
}

void TransferFunctionEditor::_onHistIndexChanged(size_t index,
                                                 const double ratio)
{
    const QString indexText = index == -1u ? "" : QString("%1").arg(index, 4);
    const QString valText =
        index == -1u ? "" : QString("%1").arg(ratio * 100.0, 4, 'f', 3);
    _ui->histogramValLabel->setText(valText);
    _ui->histogramIndexLabel->setText(indexText);
}

void TransferFunctionEditor::_publishMaterialLUT()
{
    lexis::render::MaterialLUT lut;

    lut.getDiffuse().resize(COLORSAMPLES);
    lut.getAlpha().resize(COLORSAMPLES);
    lut.getContribution().resize(COLORSAMPLES);

    const auto& reds = _controlPointsWidgets[0]->getCurve();
    const auto& greens = _controlPointsWidgets[1]->getCurve();
    const auto& blues = _controlPointsWidgets[2]->getCurve();
    const auto& alphas = _controlPointsWidgets[3]->getCurve();

    for (size_t i = 0; i < COLORSAMPLES; ++i)
    {
        lut.getDiffuse()[i] =
            lexis::render::Color(reds[i] / 255.f, greens[i] / 255.f,
                                 blues[i] / 255.f);
        lut.getAlpha()[i] = alphas[i] / 255.f;
        lut.getContribution()[i] = 1.0f;
    }

    if (!_histogram.isEmpty())
        lut.setRange(_rangeWidget->fromNormalizedRange().array);

    _controller.publish(lut);
}

namespace
{
const quint32 TF_FILE_HEADER = 0xdeadbeef;
const quint32 TF_FILE_VERSION_1 = 1; // stores control points absolutely
                                     // considering widget geometry (sigh)
const quint32 TF_FILE_VERSION_2 = 2; // stores control points normalized
const QString TF_FILE_FILTER("Transfer function's control points, *.tf");
const QString DT_FILE_FILTER("ImageVis3d compatible ascii, *.1dt");
const QString TF_FILTERS(TF_FILE_FILTER + ";;" + DT_FILE_FILTER);

QPolygonF _filterPoints(const QPolygonF& points)
{
    QPolygonF filteredPoints;
    float prevSlope = 0;
    QPointF prevPoint = points.first();
    const float epsilon = 0.001f;
    for (int i = 1; i < points.size() - 1; ++i)
    {
        const auto& currentPoint = points[i];
        const QLineF currentLine(prevPoint, currentPoint);
        const float currentSlope =
            float(currentLine.dy()) / float(currentLine.dx());

        bool change = std::abs(prevSlope - currentSlope) > epsilon;
        if (change)
        {
            const QLineF nextLine(currentPoint, points[i + 1]);
            const float nextSlope = float(nextLine.dy()) / float(nextLine.dx());
            if (std::abs(prevSlope - nextSlope) <= epsilon)
                change = false;
        }

        if (change || i == 1)
        {
            prevSlope = currentSlope;
            filteredPoints << prevPoint;
        }
        prevPoint = points[i];
    }
    filteredPoints << points.last();
    return filteredPoints;
}
}

void TransferFunctionEditor::_load()
{
    QString selectedFilter;
    const QString filename =
        QFileDialog::getOpenFileName(this, "Load transfer function", QString(),
                                     TF_FILTERS, &selectedFilter);
    if (filename.isEmpty())
        return;

    if (selectedFilter == TF_FILE_FILTER)
    {
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);

        quint32 header;
        in >> header;
        if (header != TF_FILE_HEADER)
            return;

        quint32 version;
        in >> version;
        if (version != TF_FILE_VERSION_1 && version != TF_FILE_VERSION_2)
            return;

        for (auto& widget : _controlPointsWidgets)
        {
            QPolygonF points;
            in >> points;
            if (version == TF_FILE_VERSION_1)
            {
                for (auto& point : points)
                {
                    point.setX(point.x() / widget->width());
                    point.setY(1.f - (point.y() / widget->height()));
                }
            }
            widget->setControlPoints(points);
        }
    }
    else if (selectedFilter == DT_FILE_FILTER)
    {
        std::ifstream file(filename.toStdString());
        uint32_t samples;
        file >> samples;
        if (samples != COLORSAMPLES)
            return;

        constexpr size_t nChannels = 4;
        QPolygonF points[nChannels];
        for (size_t i = 0; i < nChannels; ++i)
            points[i].resize(samples);

        for (size_t i = 0; i < samples; ++i)
        {
            for (size_t j = 0; j < nChannels; ++j)
            {
                points[j][i].rx() = i / float(samples - 1);
                file >> points[j][i].ry();
            }
        }

        for (size_t i = 0; i < nChannels; ++i)
            _controlPointsWidgets[i]->setControlPoints(
                _filterPoints(points[i]));
    }
    else
        return;

    emit transferFunctionChanged();
}

void TransferFunctionEditor::_save()
{
    QString selectedFilter;
    QString filename =
        QFileDialog::getSaveFileName(this, "Save transfer function", QString(),
                                     TF_FILTERS, &selectedFilter);
    if (selectedFilter == TF_FILE_FILTER)
    {
        if (!filename.endsWith(".tf"))
            filename.append(".tf");

        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_0);

        out << TF_FILE_HEADER << TF_FILE_VERSION_2;

        for (const auto& widget : _controlPointsWidgets)
            out << widget->getControlPoints();
    }
    else if (selectedFilter == DT_FILE_FILTER)
    {
        if (!filename.endsWith(".1dt"))
            filename.append(".1dt");

        std::ofstream file;
        file.open(filename.toStdString());
        file << COLORSAMPLES << std::endl;

        const auto& reds = _controlPointsWidgets[0]->getCurve();
        const auto& greens = _controlPointsWidgets[1]->getCurve();
        const auto& blues = _controlPointsWidgets[2]->getCurve();
        const auto& alphas = _controlPointsWidgets[3]->getCurve();

        for (size_t i = 0; i < COLORSAMPLES; ++i)
            file << reds[i] / 255.f << " " << greens[i] / 255.f << " "
                 << blues[i] / 255.f << " " << alphas[i] / 255.f << std::endl;
    }
}

void TransferFunctionEditor::_onTransferFunctionChanged()
{
    _setGradientStops();
    _publishMaterialLUT();
}

void TransferFunctionEditor::_onRangeChanged(const vmml::Vector2f range)
{
    _alphaWidget->setRange(range);
    _publishMaterialLUT();
}

void TransferFunctionEditor::_onHistogramChanged(const bool logScale)
{
    _alphaWidget->setHistogram(_histogram, logScale);
    _rangeWidget->setHistogram(_histogram, logScale);
}
}
