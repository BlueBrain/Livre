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

#pragma once

#include <livreGUI/types.h>

#include <lexis/render/Histogram.h>

#include <QWidget>
#include <array>

namespace Ui
{
class TransferFunctionEditor;
}

namespace livre
{
/** This contains all the widget for the transfer function editor. */
class TransferFunctionEditor : public QWidget
{
    Q_OBJECT

public:
    TransferFunctionEditor(Controller& controller, QWidget* parent);
    ~TransferFunctionEditor();

signals:
    void transferFunctionChanged();
    void histogramChanged(bool logScale);

private Q_SLOTS:
    void _load();
    void _save();
    void _setDefault();
    void _onHistIndexChanged(size_t index, double value);
    void _onHistogramChanged(bool logScale);
    void _onTransferFunctionChanged();
    void _onRangeChanged(vmml::Vector2f range);

private:
    void _publishMaterialLUT();
    void _setGradientStops();

    lexis::render::Histogram _histogram;
    Controller& _controller;
    Ui::TransferFunctionEditor* _ui;

    typedef std::array<ControlPointsWidget*, 4> ControlPointsWidgets;
    ControlPointsWidgets _controlPointsWidgets;
    AlphaWidget* _alphaWidget{nullptr};
    RangeWidget* _rangeWidget{nullptr};
};
}
