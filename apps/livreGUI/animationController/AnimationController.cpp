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

#include <livre/core/types.h>

#include <livreGUI/animationController/AnimationController.h>
#include <livreGUI/Controller.h>

#include <zeq/types.h>
#include <zeq/event.h>
#include <zeq/subscriber.h>
#include <zeq/publisher.h>

namespace livre
{

const size_t DEFAULT_MIN_VALUE   = 0;
const size_t DEFAULT_MAX_VALUE   = 1e4;
const size_t DEFAULT_TICK_COUNTS = 10;

/**
 * AnimationEditor defines the possible actions for the animation feature
 **/
enum AnimationAction
{
    AA_PAUSE,
    AA_PLAY,
    AA_PLAY_BACKWARDS
};

struct AnimationController::Impl
{

    Impl( AnimationController* animationController,
          Controller& controller,
          Ui_animationController& ui )
        : _ui( ui )
        , _animationController( animationController )
        , _controller( controller )
        , _startFrame( DEFAULT_MIN_VALUE )
        , _endFrame( DEFAULT_MAX_VALUE )
        , _currentFrame( 0 )
        , _action( AA_PAUSE )
        , _connected( false )
        , _onFirstFrame( true )
    {}

    ~Impl()
    {
    }

    void onFrame( const ::zeq::Event& event_ )
    {
        emit _animationController->newFrameReceived( zeq::hbp::deserializeFrame( event_ ));
    }

    void onFirstHeartbeatEvent()
    {
        emit _animationController->firstHeartBeatReceived();
    }

    void setSubscriber()
    {
        const servus::URI uri( _ui.leURI->text().toStdString( ));
        _controller.registerHandler( uri, ::zeq::hbp::EVENT_FRAME,
                                     boost::bind( &AnimationController::Impl::onFrame,
                                                  this, _1 ));
        _controller.registerHandler( uri, zeq::vocabulary::EVENT_HEARTBEAT,
                                     boost::bind(
                                         &AnimationController::Impl::onFirstHeartbeatEvent,
                                         this ));
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
            resetControls();
        }
    }

    void disconnect()
    {
        _controller.deregisterHandler( servus::URI( _ui.leURI->text().toStdString()),
                                       ::zeq::hbp::EVENT_FRAME,
                                       boost::bind( &AnimationController::Impl::onFrame,
                                                    this, _1 ));
        _connected = false;
        resetControls();
    }

    void resetControls()
    {
        _ui.btnConnect->setEnabled( !_connected );
        _ui.btnDisconnect->setEnabled( _connected );
        _ui.btnPlay->setEnabled( _connected && _action==AA_PAUSE );
        _ui.btnPause->setEnabled( _connected && _action!=AA_PAUSE );
        _ui.sldFrame->setEnabled( _connected );

        if( _action == AA_PLAY )
        {
            _ui.chbxReverse->setChecked( false );
            _ui.chbxReverse->setEnabled( false );
        }
        else if( _action == AA_PLAY_BACKWARDS )
        {
            _ui.chbxReverse->setChecked( true );
            _ui.chbxReverse->setEnabled( false );
        }
        else
            _ui.chbxReverse->setEnabled( true );
    }

    void slider( int value )
    {
        _ui.btnPlay->setEnabled( _connected && _action==AA_PAUSE );
        _ui.btnPause->setEnabled( _connected && _action!=AA_PAUSE );
        _currentFrame = value;
        _ui.lblFrame->setText( QString::number( _currentFrame ));
        publishFrame();
    }

    void play()
    {
        if( _ui.chbxReverse->checkState() == Qt::Checked )
            _action = AA_PLAY_BACKWARDS;
        else
            _action = AA_PLAY;

        resetControls();
        publishFrame();
    }

    void pause()
    {
        _action = AA_PAUSE;
        resetControls();
        publishFrame();
    }

    void onFirstHeartBeatReceived()
    {
        const servus::URI uri( _ui.leURI->text().toStdString( ));
        _controller.publish( uri, ::zeq::vocabulary::serializeRequest( ::zeq::hbp::EVENT_FRAME ));
        _controller.deregisterHandler( uri, zeq::vocabulary::EVENT_HEARTBEAT,
                                       boost::bind(
                                           &AnimationController::Impl::onFirstHeartbeatEvent,
                                           this ));
        _connected = true;
        resetControls();
    }

    void onNewFrameReceived( zeq::hbp::data::Frame frame )
    {
        _connected = true;
        _startFrame = frame.start;
        _endFrame = std::min(( int32_t )frame.end, std::numeric_limits< int32_t >::max());
        _currentFrame = frame.current;
        _ui.sldFrame->setMinimum( _startFrame );
        _ui.sldFrame->setMaximum( _endFrame );

        if( _onFirstFrame )
        {
            if( frame.delta > 0 )
            {
                _onFirstFrame = false;
                _action = AA_PLAY;
                resetControls();
            }
            else if( frame.delta < 0 )
            {
                _onFirstFrame = false;
                _action = AA_PLAY_BACKWARDS;
                resetControls();
            }
        }

        // Update slider value only when animation is active
        if(( _action == AA_PLAY ) || ( _action == AA_PLAY_BACKWARDS ))
        {
            _ui.sldFrame->blockSignals( true );
            _ui.sldFrame->setValue( _currentFrame );
            _ui.lblFrame->setText( QString::number( _currentFrame ));
            _ui.sldFrame->blockSignals( false );
        }
    }

    void publishFrame() const
    {
        if( _connected )
        {
            const ::zeq::hbp::data::Frame frame(
                        _startFrame, _currentFrame, _endFrame, getDeltaFromAction());
            _controller.publish( servus::URI( _ui.leURI->text().toStdString()),
                                 ::zeq::hbp::serializeFrame( frame ));
        }
    }

    int getDeltaFromAction() const
    {
        int delta = 0;
        switch( _action )
        {
        case AA_PLAY:
            delta = 1;
            break;
        case AA_PLAY_BACKWARDS:
            delta = -1;
            break;
        default:
            break;
        }
        return delta;
    }

public:

    Ui_animationController& _ui;
    AnimationController* _animationController;
    Controller& _controller;
    uint32_t        _startFrame;
    uint32_t        _endFrame;
    uint32_t        _currentFrame;
    AnimationAction _action;
    bool            _connected;
    bool            _onFirstFrame;
};

AnimationController::AnimationController( Controller& controller,
                                          QWidget* parentWgt )
    : QWidget( parentWgt )
    , _impl( new AnimationController::Impl( this, controller, _ui ))
{
    qRegisterMetaType< ::zeq::hbp::data::Frame >("::zeq::hbp::data::Frame");

    _ui.setupUi( this );
    connect( _ui.sldFrame, SIGNAL( valueChanged( int )), this, SLOT( _slider( int )));
    connect( _ui.btnPlay, SIGNAL( pressed()), this, SLOT( _play()));
    connect( _ui.btnPause, SIGNAL( pressed()), this, SLOT( _pause()));
    connect( _ui.btnConnect, SIGNAL( pressed()), this, SLOT( _connect()));
    connect( _ui.btnDisconnect, SIGNAL( pressed()), this, SLOT( _disconnect()));
    connect( this, &AnimationController::newFrameReceived,
             this, &AnimationController::_onNewFrameReceived,
             Qt::QueuedConnection );
    connect( this, &AnimationController::firstHeartBeatReceived,
             this, &AnimationController::_onFirstHeartBeatReceived,
             Qt::QueuedConnection );
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

void AnimationController::_slider( int value )
{
    _impl->slider( value );
}

void AnimationController::_play()
{
    _impl->play();
}

void AnimationController::_pause()
{
    _impl->pause();
}

void AnimationController::_onNewFrameReceived( zeq::hbp::data::Frame frame )
{
    _impl->onNewFrameReceived( frame );
}

void AnimationController::_onFirstHeartBeatReceived()
{
    _impl->onFirstHeartBeatReceived();
}

}

