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
#include <livreGUI/ui_RenderParametersController.h>
#include <livreGUI/Controller.h>


namespace livre
{
struct RenderParametersController::Impl
{
    Impl( RenderParametersController* parent, Controller& controller )
        : _parent( parent )
        , _controller( controller )
    {
        _ui.setupUi( parent );

        _params.setUpdatedFunction(
            [parent]() { emit parent->paramsUpdated(); } );
        _controller.subscribe( _params );
    }

    ~Impl()
    {
        _controller.unsubscribe( _params );
    }

    void onParamsUpdated()
    {
        _ui.maxLODSpinBox->blockSignals( true );
        _ui.maxLODSpinBox->setValue( _params.getMaxLOD( ));
        _ui.maxLODSpinBox->blockSignals( false );

        _ui.minLODSpinBox->blockSignals( true );
        _ui.minLODSpinBox->setValue( _params.getMinLOD( ));
        _ui.minLODSpinBox->blockSignals( false );

        _ui.screenSpaceErrorDoubleSpinBox->blockSignals( true );
        _ui.screenSpaceErrorDoubleSpinBox->setValue( _params.getSSE( ));
        _ui.screenSpaceErrorDoubleSpinBox->blockSignals( false );

        _ui.synchronousCheckBox->blockSignals( true );
        _ui.synchronousCheckBox->setChecked( _params.getSynchronousMode( ));
        _ui.synchronousCheckBox->blockSignals( false );

        _ui.samplesPerPixelSpinBox->blockSignals( true );
        _ui.samplesPerPixelSpinBox->setValue( _params.getSamplesPerPixel( ));
        _ui.samplesPerPixelSpinBox->blockSignals( false );

        _ui.samplesPerRaySpinBox->blockSignals( true );
        _ui.samplesPerRaySpinBox->setValue( _params.getSamplesPerRay( ));
        _ui.samplesPerRaySpinBox->blockSignals( false );
    }

    void publish()
    {
        _params.setMaxLOD( _ui.maxLODSpinBox->value( ));
        _params.setMinLOD( _ui.minLODSpinBox->value( ));
        _params.setSSE( _ui.screenSpaceErrorDoubleSpinBox->value( ));
        _params.setSynchronousMode( _ui.synchronousCheckBox->isChecked( ));
        _params.setSamplesPerPixel( _ui.samplesPerPixelSpinBox->value( ));
        _params.setSamplesPerRay( _ui.samplesPerRaySpinBox->value( ));
        _controller.publish( _params );
    }

    Ui::renderParametersController _ui;
    RenderParametersController* _parent;
    Controller& _controller;
    zerobuf::VolumeRendererParameters _params;
};

RenderParametersController::RenderParametersController( Controller& controller,
                                                        QWidget* parentWgt )
    : QWidget( parentWgt )
    , _impl( new RenderParametersController::Impl( this, controller ))
{
    connect( this, &RenderParametersController::paramsUpdated,
             this, &RenderParametersController::onParamsUpdated );

    connect( _impl->_ui.maxLODSpinBox,
             static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
             [this]( int ) { _impl->publish(); });
    connect( _impl->_ui.minLODSpinBox,
             static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
             [this]( int ) { _impl->publish(); });
    connect( _impl->_ui.screenSpaceErrorDoubleSpinBox,
             static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
             [this]( double ) { _impl->publish(); });
    connect( _impl->_ui.synchronousCheckBox, &QCheckBox::stateChanged,
             [this]( int ) { _impl->publish(); });
    connect( _impl->_ui.samplesPerPixelSpinBox,
             static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
             [this]( int ) { _impl->publish(); });
    connect( _impl->_ui.samplesPerRaySpinBox,
             static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
             [this]( int ) { _impl->publish(); });
}

RenderParametersController::~RenderParametersController( )
{
}

void RenderParametersController::onParamsUpdated()
{
    _impl->onParamsUpdated();
}

}
