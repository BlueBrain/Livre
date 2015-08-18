/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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
#include <zeq/hbp/vocabulary.h>

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
        : _publisher( 0 )
        , _subscriber( 0 )
        , _ui( ui )
        , _animationController( animationController )
        , _controller( controller )
        , _startFrame( DEFAULT_MIN_VALUE )
        , _endFrame( DEFAULT_MAX_VALUE )
        , _currentFrame( 0 )
        , _action( AA_PAUSE )
        , _connected( false )
        , _initialFrameReceived( false )
    {
    }

    ~Impl()
    {
    }

    void onFrame( const ::zeq::Event& event_ )
    {
        _initialFrameReceived = true;
        const zeq::hbp::data::Frame frame
                = zeq::hbp::deserializeFrame( event_ );
        _startFrame = frame.start;
        _endFrame  = frame.end;
        _currentFrame = frame.current;
        _ui.sldFrame->setMinimum( _startFrame );
        _ui.sldFrame->setMaximum( _endFrame );

        // Update slider value only when animation is active
        if( _action == AA_PLAY )
            _ui.sldFrame->setValue( _currentFrame );
    }

    void setSubscriber()
    {
        const QString& uriStr = _ui.leURI->text();
        const servus::URI uri( uriStr.toStdString( ));

        _subscriber = _controller.getSubscriber( uri );
        _subscriber->registerHandler( ::zeq::hbp::EVENT_FRAME,
                                      boost::bind( &AnimationController::Impl::onFrame,
                                                   this, _1 ));
    }

    void setPublisher()
    {
        const QString& uriStr = _ui.leURI->text();
        const servus::URI uri( uriStr.toStdString( ));

        _publisher = _controller.getPublisher( uri );
    }

    void connect()
    {
        try
        {
            setSubscriber();
            setPublisher();

            // Trying for 5 attempts to get a Frame event from the renderer. Only consider
            // the connection as valid if the Frame event was received.
            size_t attempts( 5 );
            while( !_initialFrameReceived && attempts>0 )
            {
                _publisher->publish(
                            ::zeq::vocabulary::serializeRequest(::zeq::hbp::EVENT_FRAME));
                sleep(1);
                --attempts;
            }
            if( _initialFrameReceived )
            {
                _connected = true;
                resetControls();
            }
            else
            {
                LBERROR << "Failed to get frame information from renderer" << std::endl;
                _subscriber = 0;
                _publisher = 0;
            }
        }
        catch( const std::exception& error )
        {
            LBERROR << "Error:" << error.what() << std::endl;
            _connected = false;
            resetControls();
            _subscriber = 0;
            _publisher = 0;
        }
    }

    void disconnect()
    {
        _subscriber->deregisterHandler( ::zeq::hbp::EVENT_FRAME );
        _subscriber = 0;
        _publisher = 0;
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
    }

    void slider( int value )
    {
        _action = AA_PAUSE;
        _currentFrame = value;
        _ui.lblFrame->setText( QString::number( _currentFrame ));
        publishFrame();
    }

    void play()
    {
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

    void publishFrame() const
    {
        if( _publisher )
        {
            const ::zeq::hbp::data::Frame frame(
                        _startFrame, _currentFrame, _endFrame, getDeltaFromAction());
            _publisher->publish( ::zeq::hbp::serializeFrame( frame ));
        }
    }

    int getDeltaFromAction()
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

    zeq::Publisher* _publisher;
    zeq::Subscriber* _subscriber;
    Ui_animationController& _ui;
    AnimationController* _animationController;
    Controller& _controller;

private:

    uint32_t        _startFrame;
    uint32_t        _endFrame;
    uint32_t        _currentFrame;
    AnimationAction _action;
    bool            _connected;
    bool            _initialFrameReceived;

};

AnimationController::AnimationController( Controller& controller,
                                          QWidget* parentWgt )
    : QWidget( parentWgt )
    , _impl( new AnimationController::Impl( this, controller, _ui ))
{
    _ui.setupUi( this );
    connect( _ui.sldFrame, SIGNAL( valueChanged( int )), this, SLOT( _slider( int )));
    connect( _ui.btnPlay, SIGNAL( pressed()), this, SLOT( _play()));
    connect( _ui.btnPause, SIGNAL( pressed()), this, SLOT( _pause()));
    connect( _ui.btnConnect, SIGNAL( pressed()), this, SLOT( _connect()));
    connect( _ui.btnDisconnect, SIGNAL( pressed()), this, SLOT( _disconnect()));
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

}
