/* Copyright (c) 2015-2016, EPFL/Blue Brain Project
 *                          ahmet.bilgili@epfl.ch
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

#include "ClipPlanesController.h"

#include <livreGUI/Controller.h>
#include <livreGUI/ui_ClipPlanesController.h>

#include <livre/core/render/ClipPlanes.h>

namespace livre
{

struct ClipPlanesController::Impl
{
    Impl( ClipPlanesController* parent, Controller& controller )
        : _parent( parent )
        , _controller( controller )
    {
        _ui.setupUi( parent );
        QWidget *slidersWidget = _ui.clipPlanesWidget;
        slidersWidget->setEnabled( false);
        _clipPlanes.clear();

        parent->connect( parent, &ClipPlanesController::clipPlanesReceived,
                         [&](){ clipPlanesReceived( );});

        slidersWidget->connect( _ui.clipPlanesCheckBox, &QCheckBox::stateChanged,
                 [&]( int value )
                 {
                    _ui.clipPlanesWidget->setEnabled( value != 0 );
                    if( value == 0 )
                    {
                        ClipPlanes planes;
                        planes.clear();
                        _controller.publish( planes );
                    }
                    else
                    {
                        if( _clipPlanes.isEmpty( ))
                            _clipPlanes.reset();
                        _controller.publish( _clipPlanes );
                    }
                 });

        slidersWidget->connect( _ui.clipPlanesResetButton, &QPushButton::clicked,
                 [&](bool)
                 {
                    _clipPlanes.reset();
                    ClipPlanes planes;
                    updateClipPlanesUi( planes );
                    _controller.publish( planes );
                 });

        _clipPlaneSliders.push_back( _ui.posXSlider );
        _clipPlaneSliders.push_back( _ui.negXSlider );
        _clipPlaneSliders.push_back( _ui.posYSlider );
        _clipPlaneSliders.push_back( _ui.negYSlider );
        _clipPlaneSliders.push_back( _ui.posZSlider );
        _clipPlaneSliders.push_back( _ui.negZSlider );

        _clipPlaneSpinBoxes.push_back( _ui.posXSpinBox );
        _clipPlaneSpinBoxes.push_back( _ui.negXSpinBox );
        _clipPlaneSpinBoxes.push_back( _ui.posYSpinBox );
        _clipPlaneSpinBoxes.push_back( _ui.negYSpinBox );
        _clipPlaneSpinBoxes.push_back( _ui.posZSpinBox );
        _clipPlaneSpinBoxes.push_back( _ui.negZSpinBox );

        for( QSlider* slider: _clipPlaneSliders )
        {
            slidersWidget->connect( slider, &QSlider::valueChanged,
                     [&]( int ) { updateClipPlanesSpinBoxes(); });
        }

        for( QDoubleSpinBox* spinBox: _clipPlaneSpinBoxes )
        {
            slidersWidget->connect( spinBox,
                    static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                     [&]( double ) { updateClipPlanesSliders(); });
        }

        controller.subscribe( _clipPlanes );
        _clipPlanes.registerDeserializedCallback(
                    [&]{
                          emit _parent->clipPlanesReceived();
                          _clipPlanes.registerDeserializedCallback( nullptr );
                       });
    }

    ~Impl()
    {
        _controller.unsubscribe( _clipPlanes );
    }

    void updateClipPlanesSliders()
    {
        if( _clipPlanes.isEmpty( ))
            return;

        SpinBoxes::const_iterator it = _clipPlaneSpinBoxes.begin();
        bool changed = false;
        size_t i = 0;
        for( QSlider* slider: _clipPlaneSliders )
        {
            const QDoubleSpinBox* spinBox = *it;
            // Map spinbox value between 0 and slider maximum
            const double spinBoxValue = spinBox->value();
            const double range = spinBox->maximum() - spinBox->minimum();
            const int value = ( spinBoxValue - spinBox->minimum( )) / range * slider->maximum();
            if( slider->value() != value )
            {
                slider->setValue( value );
                _clipPlanes.getPlanes()[ i ].setD( i % 2 == 0 ? spinBoxValue : -spinBoxValue );
                changed = true;
            }
            ++it;
            ++i;
        }

        if( changed )
           _controller.publish( _clipPlanes );
    }

    void updateClipPlanesSpinBoxes()
    {
        if( _clipPlanes.isEmpty( ))
            return;

        Sliders::const_iterator it = _clipPlaneSliders.begin();
        bool changed = false;
        size_t i = 0;
        for( QDoubleSpinBox* spinBox: _clipPlaneSpinBoxes )
        {
            const QSlider* slider = *it;
            const double range = spinBox->maximum() - spinBox->minimum();
            const double value =
                    ((double)slider->value() / (double)slider->maximum( )) * range +
                    spinBox->minimum();
            if( spinBox->value() != value )
            {
                spinBox->setValue( value );
                _clipPlanes.getPlanes()[ i ].setD( i % 2 == 0 ? value : -value );
                changed = true;
            }
            ++it;
            ++i;
        }

        if( changed )
            _controller.publish( _clipPlanes );
    }

    void clipPlanesReceived()
    {
        updateClipPlanesUi( _clipPlanes );
    }

    void updateClipPlanesUi( ClipPlanes& clipPlanes )
    {
        size_t i = 0;
        auto& planes = clipPlanes.getPlanes();
        for( QDoubleSpinBox* spinBox: _clipPlaneSpinBoxes )
        {
            if( i == planes.size())
                break;

            ::lexis::render::Plane& plane = planes[ i ];
            spinBox->setValue(( i % 2 ) == 0  ? plane.getD() : -plane.getD( ));
            ++i;
        }
    }

    Ui::clipPlanesController _ui;
    ClipPlanesController* _parent;
    Controller& _controller;
    ClipPlanes _clipPlanes;

    typedef std::vector< QSlider* > Sliders;
    typedef std::vector< QDoubleSpinBox* > SpinBoxes;

    Sliders _clipPlaneSliders;
    SpinBoxes _clipPlaneSpinBoxes;
};

ClipPlanesController::ClipPlanesController( Controller& controller,
                                            QWidget* parentWgt )
    : QWidget( parentWgt )
    , _impl( new ClipPlanesController::Impl( this, controller ))
{
}

ClipPlanesController::~ClipPlanesController( )
{
}

}
