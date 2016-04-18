/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                          Jafet Villafranca <jafet.villafrancadiaz@epfl.ch>
 *                          Raphael Dumusc <raphael.dumusc@epfl.ch>
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

#include "MainWindow.h"
#include <livreGUI/ui_MainWindow.h>

#include "animationController/AnimationController.h"
#include "progress/Progress.h"
#include "renderParametersController/RenderParametersController.h"
#include "tfEditor/TransferFunctionEditor.h"
#include "Controller.h"

#ifdef LIVRE_USE_MONSTEER
#  include <monsteer/qt/SteeringWidget.h>
#endif

namespace livre
{
struct MainWindow::Impl
{
    Impl( MainWindow* parent, Controller& controller )
    {
        _ui.setupUi( parent );

        parent->setCentralWidget( new TransferFunctionEditor( controller ));

#ifdef LIVRE_USE_MONSTEER
        _ui.simulationDockWidget->setWidget( new monsteer::qt::SteeringWidget );
#else
        _ui.simulationDockWidget->setHidden( true );
#endif

        _ui.animationDockWidget->setWidget(
            new AnimationController( controller ));

        _ui.progressDockWidget->setWidget( new Progress( controller ));
        _ui.progressDockWidget->setHidden( true );

        _ui.renderParametersDockWidget->setWidget( new RenderParametersController( controller ));

        if( !_ui.simulationDockWidget->isHidden( ))
            parent->tabifyDockWidget( _ui.simulationDockWidget,
                                      _ui.renderParametersDockWidget);
    }

    ~Impl(){}

private:
    Ui::MainWindow _ui;
};

MainWindow::MainWindow( Controller& controller, QWidget* parent_ )
    : QMainWindow( parent_ )
    , _impl( new Impl( this, controller ))
{}

MainWindow::~MainWindow()
{}

}
