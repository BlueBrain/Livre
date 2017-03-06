/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
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
#include <livreGUI/volumeRendererParameters.h>

namespace livre
{
struct RenderParametersController::Impl
{
    Impl(RenderParametersController* parent, Controller& controller)
        : _parent(parent)
        , _controller(controller)
    {
        _ui.setupUi(parent);

        parent->connect(_ui.maxLODSpinBox, static_cast<void (QSpinBox::*)(int)>(
                                               &QSpinBox::valueChanged),
                        [&](int value) {
                            _params.setMaxLod(value);
                            publish();
                        });
        parent->connect(_ui.minLODSpinBox, static_cast<void (QSpinBox::*)(int)>(
                                               &QSpinBox::valueChanged),
                        [&](int value) {
                            _params.setMinLod(value);
                            publish();
                        });
        parent->connect(_ui.screenSpaceErrorDoubleSpinBox,
                        static_cast<void (QDoubleSpinBox::*)(double)>(
                            &QDoubleSpinBox::valueChanged),
                        [&](double value) {
                            _params.setScreenSpaceError(value);
                            publish();
                        });
        parent->connect(_ui.synchronousCheckBox, &QCheckBox::stateChanged,
                        [&](int value) {
                            _params.setSynchronousMode(value != 0);
                            publish();
                        });
        parent->connect(_ui.samplesPerPixelSpinBox,
                        static_cast<void (QSpinBox::*)(int)>(
                            &QSpinBox::valueChanged),
                        [&](int value) {
                            _params.setSamplesPerPixel(value);
                            publish();
                        });
        parent->connect(_ui.samplesPerRaySpinBox,
                        static_cast<void (QSpinBox::*)(int)>(
                            &QSpinBox::valueChanged),
                        [&](int value) {
                            _params.setSamplesPerRay(value);
                            publish();
                        });
        parent->connect(_ui.showAxesCheckBox, &QCheckBox::stateChanged,
                        [&](int value) {
                            _params.setShowAxes(value != 0);
                            publish();
                        });

        parent->connect(&_params, &v1::VolumeRendererParameters::maxLodChanged,
                        [&](uint32_t value) {
                            _ui.maxLODSpinBox->setValue(value);
                        });
        parent->connect(&_params, &v1::VolumeRendererParameters::minLodChanged,
                        [&](uint32_t value) {
                            _ui.minLODSpinBox->setValue(value);
                        });
        parent->connect(&_params,
                        &v1::VolumeRendererParameters::screenSpaceErrorChanged,
                        [&](float value) {
                            _ui.screenSpaceErrorDoubleSpinBox->setValue(value);
                        });
        parent->connect(&_params,
                        &v1::VolumeRendererParameters::synchronousModeChanged,
                        [&](bool value) {
                            _ui.synchronousCheckBox->setChecked(value);
                        });
        parent->connect(&_params,
                        &v1::VolumeRendererParameters::samplesPerPixelChanged,
                        [&](uint32_t value) {
                            _ui.samplesPerPixelSpinBox->setValue(value);
                        });
        parent->connect(&_params,
                        &v1::VolumeRendererParameters::samplesPerRayChanged,
                        [&](uint32_t value) {
                            _ui.samplesPerRaySpinBox->setValue(value);
                        });
        parent->connect(&_params,
                        &v1::VolumeRendererParameters::showAxesChanged,
                        [&](bool value) {
                            _ui.showAxesCheckBox->setChecked(value);
                        });

        _controller.subscribe(_params);
    }

    ~Impl() { _controller.unsubscribe(_params); }
    void publish() { _controller.publish(_params); }
    Ui::renderParametersController _ui;
    RenderParametersController* _parent;
    Controller& _controller;
    v1::VolumeRendererParameters _params;
};

RenderParametersController::RenderParametersController(Controller& controller,
                                                       QWidget* parentWgt)
    : QWidget(parentWgt)
    , _impl(new RenderParametersController::Impl(this, controller))
{
}

RenderParametersController::~RenderParametersController()
{
}
}
