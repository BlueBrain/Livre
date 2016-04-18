/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Cyrille Favreau <cyrille.favreau@epfl.ch>
 *                          Grigori Chevtchenko <grigori.chevtchenko>
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

#include "AnimationController.h"
#include "../Controller.h"
#include <livreGUI/ui_AnimationController.h>
#include <livre/core/types.h>

#include <zeroeq/event.h>

namespace livre
{

struct AnimationController::Impl
{
    Impl( AnimationController* animationController,
          Controller& controller)
        : _animationController( animationController )
        , _controller( controller )
        , _connected( false )
        , _onFirstFrame( true )
    {
        _ui.setupUi( _animationController );
        connect();
    }

    ~Impl()
    {
        disconnect();
    }

    void onFrame( const ::zeroeq::Event& event_ )
    {
        emit _animationController->newFrameReceived( zeroeq::hbp::deserializeFrame( event_ ));
    }

    void setSubscriber()
    {
        _controller.registerHandler( ::zeroeq::hbp::EVENT_FRAME,
                                     std::bind( &AnimationController::Impl::onFrame,
                                                  this, std::placeholders::_1 ));
    }

    void connect()
    {
         _onFirstFrame = true;
        try
        {
            setSubscriber();
        }
        catch( const std::exception& error )
        {
            LBERROR << "Error:" << error.what() << std::endl;
            _connected = false;
        }
        resetControls();
    }

    void disconnect()
    {
        _controller.deregisterHandler( ::zeroeq::hbp::EVENT_FRAME );
        _connected = false;
        resetControls();
    }

    void resetControls()
    {
        const bool followSimulation = _ui.chbxFollow->isChecked();
        _ui.chbxReverse->setEnabled( !followSimulation );
        _ui.btnPlay->setEnabled( !followSimulation );
        _ui.sldFrame->setEnabled( !followSimulation );
        _animationController->setEnabled( _connected );
    }

    bool isPlaying() const
    {
        return _ui.btnPlay->text() == "Pause";
    }

    void setPlaying( const bool enable )
    {
        _ui.btnPlay->setText( enable ? "Pause" : "Play" );
    }

    void onNewFrameReceived( zeroeq::hbp::data::Frame frame )
    {
        _connected = true;

        const int32_t int32Max = std::numeric_limits<int32_t>::max();
        const int32_t startFrame = std::min( (int32_t)frame.start, int32Max );
        const int32_t endFrame = std::min( (int32_t)frame.end, int32Max );

        // Ignore events with invalid frame range, observed when a remote data
        // source is not yet connected
        if( startFrame >= endFrame )
            return;

        // QSlider has no reliable signal for user only input.
        // valueChange() is always fired and sliderMoved() does not signal on
        // keyboard input. So block signal emission when setting the value
        // programatically.
        _ui.sldFrame->blockSignals( true );
        _ui.startFrameSpinBox->blockSignals( true );
        _ui.currentFrameSpinBox->blockSignals( true );
        _ui.endFrameSpinBox->blockSignals( true );

        _ui.sldFrame->setMinimum( startFrame );
        _ui.startFrameSpinBox->setValue( startFrame );

        _ui.sldFrame->setMaximum( endFrame - 1 );
        _ui.endFrameSpinBox->setValue( endFrame - 1 );

        _ui.sldFrame->setValue( frame.current );
        _ui.currentFrameSpinBox->setValue( frame.current );

        _ui.sldFrame->blockSignals( false );
        _ui.startFrameSpinBox->blockSignals( false );
        _ui.currentFrameSpinBox->blockSignals( false );
        _ui.endFrameSpinBox->blockSignals( false );

        if( _onFirstFrame )
        {
            _onFirstFrame = false;
            resetControls();
        }

        setPlaying( frame.delta != 0 );
        _ui.chbxFollow->setChecked( frame.delta == LATEST_FRAME );
        _ui.chbxReverse->setChecked( frame.delta < 0 );

        if( endFrame - startFrame <= 1 )
            _animationController->setDisabled( true );
        else
            _animationController->setEnabled( true );
    }

    void publishFrame() const
    {
        if( _connected )
        {
            const ::zeroeq::hbp::data::Frame frame( _ui.sldFrame->minimum(),
                                                    _ui.sldFrame->value(),
                                                    _ui.sldFrame->maximum() + 1,
                                                    getFrameDelta( ));
            _controller.publish( ::zeroeq::hbp::serializeFrame( frame ));
        }
    }

    int getFrameDelta() const
    {
        if( !isPlaying( ))
            return 0;

        if( _ui.chbxFollow->isChecked( ))
            return LATEST_FRAME;

        if( _ui.chbxReverse->isChecked( ))
            return -1;

        return 1;
    }

public:
    Ui::animationController _ui;
    AnimationController* _animationController;
    Controller& _controller;
    bool _connected;
    bool _onFirstFrame;
};

AnimationController::AnimationController( Controller& controller,
                                          QWidget* parentWgt )
    : QWidget( parentWgt )
    , _impl( new AnimationController::Impl( this, controller ))
{
    qRegisterMetaType< ::zeroeq::hbp::data::Frame >( "::zeroeq::hbp::data::Frame" );

    connect( _impl->_ui.sldFrame, &QSlider::valueChanged,
             this, &AnimationController::_onFrameChanged );
    connect( _impl->_ui.sldFrame, &QSlider::rangeChanged,
             this, &AnimationController::_onFrameChanged );

    connect( _impl->_ui.startFrameSpinBox,
             static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
             _impl->_ui.sldFrame, &QSlider::setMinimum );
    connect( _impl->_ui.endFrameSpinBox,
             static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
             _impl->_ui.sldFrame, &QSlider::setMaximum );

    connect( _impl->_ui.btnPlay, SIGNAL( pressed( )),
             this, SLOT( _togglePlayPause( )));
    connect( _impl->_ui.chbxFollow, SIGNAL( stateChanged( int )),
             this, SLOT( _setFollow( int )));
    connect( this, &AnimationController::newFrameReceived,
             this, &AnimationController::_onNewFrameReceived,
             Qt::QueuedConnection );

    _impl->_ui.chbxReverse->setVisible( false ); // temporarily hidden
}

AnimationController::~AnimationController( )
{
    delete _impl;
}

void AnimationController::_connect()
{
    _impl->connect();
}

void AnimationController::_disconnect()
{
    _impl->disconnect();
}

void AnimationController::_onFrameChanged()
{
    _impl->publishFrame();
}

void AnimationController::_togglePlayPause()
{
    _impl->setPlaying( !_impl->isPlaying( ));
    _impl->resetControls();
    _impl->publishFrame();
}

void AnimationController::_setFollow( int on )
{
    _impl->setPlaying( on );
    _impl->resetControls();
    _impl->publishFrame();
}

void AnimationController::_onNewFrameReceived( zeroeq::hbp::data::Frame frame )
{
    _impl->onNewFrameReceived( frame );
}

}
