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

#ifndef _TransferFunctionEditor_h_
#define _TransferFunctionEditor_h_

#include <livreGUI/types.h>
#include <livre/core/data/Histogram.h>

#include <array>
#include <QWidget>

namespace Ui { class TransferFunctionEditor; }

namespace livre
{
class ColorMapWidget;

/** This contains all the widget for the transfer function editor. */
class TransferFunctionEditor: public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor of TransferFunctionEditor.
     * @param controller The ZeroEQ publish/receive controller
     * @param tfParentWidget The parent GUI widget
     */
    TransferFunctionEditor( livre::Controller& controller,
                            QWidget* tfParentWidget = 0 );
    ~TransferFunctionEditor();

signals:
    void transferFunctionChanged();
    void histogramChanged( bool logScale );

private Q_SLOTS:
    void _clear();
    void _load();
    void _save();
    void _setDefault();
    void _onHistIndexChanged( size_t index, double value);
    void _onHistogramChanged( bool logScale );
    void _onTransferFunctionChanged();

private:
    void _publishTransferFunction();
    void _setGradientStops();
    void _setHistogram();

    livre::Histogram _histogram;
    livre::Controller& _controller;
    Ui::TransferFunctionEditor* _ui;

    typedef std::array< ColorMapWidget*, 4 > ColorWidgets;
    ColorWidgets _colorWidgets;
};

}

#endif // _TransferFunctionEditor_h_
