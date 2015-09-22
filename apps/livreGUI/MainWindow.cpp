/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <livreGUI/animationController/AnimationController.h>
#include <livreGUI/tfEditor/TransferFunctionEditor.h>
#ifdef LIVRE_USE_MONSTEER
#include <livreGUI/stimuliController/StimuliController.h>
#endif

MainWindow::MainWindow( livre::Controller& controller, QWidget* parent_ )
    : QMainWindow( parent_ )
    , _ui( new Ui::MainWindow )
{
    _ui->setupUi( this );

    setCentralWidget( new livre::TransferFunctionEditor( controller ));

#ifdef LIVRE_USE_MONSTEER
    _ui->simulationDockWidget->setWidget(
                new livre::StimuliController( controller ));
#endif

    _ui->animationDockWidget->setWidget(
                new livre::AnimationController( controller ));
}

MainWindow::~MainWindow()
{
    delete _ui;
}
