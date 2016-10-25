/* Copyright (c) 2015-2016, EPFL/Blue Brain Project
 *                          Daniel.Nachbaur@epfl.ch
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

#include "RenderParametersController.h"

#include <livreGUI/Controller.h>
#include <livreGUI/ui_RenderParametersController.h>
#include <livreGUI/rendererParameters.h>

namespace livre
{
struct RenderParametersController::Impl
{
    Impl( RenderParametersController* parent, Controller& controller )
        : _parent( parent )
        , _controller( controller )
    {
        _ui.setupUi( parent );

        parent->connect( _ui.maxLODSpinBox,
                 static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                 [&]( int value ) { _params.setMaxLOD( value ); publish(); });
        parent->connect( _ui.minLODSpinBox,
                 static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                 [&]( int value ) { _params.setMinLOD( value ); publish(); });
        parent->connect( _ui.screenSpaceErrorDoubleSpinBox,
                 static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                 [&]( double value ) { _params.setSSE( value ); publish(); });
        parent->connect( _ui.synchronousCheckBox, &QCheckBox::stateChanged,
                 [&]( int value ) { _params.setSynchronousMode( value != 0 ); publish(); });
        parent->connect( _ui.samplesPerPixelSpinBox,
                 static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                 [&]( int value ) { _params.setSamplesPerPixel( value ); publish(); });
        parent->connect( _ui.samplesPerRaySpinBox,
                 static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                 [&]( int value ) { _params.setSamplesPerRay( value ); publish(); });

        parent->connect( &_params, &zerobuf::RendererParameters::maxLODChanged,
                 [&]( uint32_t value ) { _ui.maxLODSpinBox->setValue( value ); });
        parent->connect( &_params, &zerobuf::RendererParameters::minLODChanged,
                 [&]( uint32_t value ) { _ui.minLODSpinBox->setValue( value ); });
        parent->connect( &_params, &zerobuf::RendererParameters::SSEChanged,
                 [&]( float value ) { _ui.screenSpaceErrorDoubleSpinBox->setValue( value ); });
        parent->connect( &_params, &zerobuf::RendererParameters::synchronousModeChanged,
                 [&]( bool value ) { _ui.synchronousCheckBox->setChecked( value ); });
        parent->connect( &_params, &zerobuf::RendererParameters::samplesPerPixelChanged,
                 [&]( uint32_t value ) { _ui.samplesPerPixelSpinBox->setValue( value ); });
        parent->connect( &_params, &zerobuf::RendererParameters::samplesPerRayChanged,
                 [&]( uint32_t value ) { _ui.samplesPerRaySpinBox->setValue( value ); });

        _controller.subscribe( _params );
    }

    ~Impl()
    {
        _controller.unsubscribe( _params );
    }

    void publish()
    {
        _controller.publish( _params );
    }

    Ui::renderParametersController _ui;
    RenderParametersController* _parent;
    Controller& _controller;
    zerobuf::RendererParameters _params;
};

RenderParametersController::RenderParametersController( Controller& controller,
                                                        QWidget* parentWgt )
    : QWidget( parentWgt )
    , _impl( new RenderParametersController::Impl( this, controller ))
{
}

RenderParametersController::~RenderParametersController( )
{
}

}
