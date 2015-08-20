/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Marwan Abdellah <marwan.abdellah@epfl.ch>
 *                     Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#include <QMainWindow>
#include <livreGUI/editor/ColorMapWidget.h>
#include <livreGUI/editor/GradientRenderer.h>
#include <QResizeEvent>
#include <livreGUI/Controller.h>

#include <zeq/publisher.h>

namespace Ui
{
class TransferFunctionEditor;
}
namespace livre
{

/**
 * This contains all the widget for the transfert function editor.
 */
class TransferFunctionEditor: public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor of TransferFunctionEditor.
     * @param conroller The controller used to receive/publish transfer fuction
     * data.
     * @param tfParentWidget Parent widget.
     */
    explicit TransferFunctionEditor( livre::Controller& conroller, QWidget* tfParentWidget = 0 );
    ~TransferFunctionEditor();

    /**
     * Set the gradient stops for colormap
     * @param stops The gradient stops.
     */
    void setColorMapStops( const QGradientStops& stops );

signals:
    void gradientStopsChanged( const QGradientStops& stops );

private Q_SLOTS:

    void _clear();
    void _setDefault();
    void _pointsUpdated();
    void _connect();
    void _disconnect();

private:

    void _publishTransferFunction();

    livre::Controller& _controller;
    Ui::TransferFunctionEditor *ui;
    bool _isConnected;
    ColorMapWidget* _redWidget;
    ColorMapWidget* _greenWidget;
    ColorMapWidget* _blueWidget;
    ColorMapWidget* _alphaWidget;
    GradientRenderer* _gradientRenderer;
};

}

#endif // _TransferFunctionEditor_h_
