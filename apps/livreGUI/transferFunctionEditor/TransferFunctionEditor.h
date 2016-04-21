/* Copyright (c) 2015-2016, EPFL/Blue Brain Project
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
#include <livre/core/render/TransferFunction1D.h>
#include <lunchbox/monitor.h> // member
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
    void gradientStopsChanged( const QGradientStops& stops );
    void transferFunctionChanged();

private Q_SLOTS:

    void _clear();
    void _load();
    void _save();
    void _setDefault();
    void _pointsUpdated();
    void _onTransferFunctionChanged();

private:

    void _publishTransferFunction();
    void _onTransferFunction();

    livre::TransferFunction1D _lut;
    livre::Controller& _controller;
    Ui::TransferFunctionEditor* _ui;
    ColorMapWidget* _redWidget;
    ColorMapWidget* _greenWidget;
    ColorMapWidget* _blueWidget;
    ColorMapWidget* _alphaWidget;
};

}

#endif // _TransferFunctionEditor_h_
