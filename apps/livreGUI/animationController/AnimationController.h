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


#ifndef _AnimationController_h_
#define _AnimationController_h_

#include <livreGUI/types.h>

#include <zeq/hbp/vocabulary.h>

#include <QWidget>

namespace livre
{

/**
 * AnimationController is the Widget for playing/pausing animations as well as
 * positionning animation to a specific frame
 **/
class AnimationController : public QWidget
{
    Q_OBJECT

public:

    /**
     * @param controller The GUI connection to zeq world.
     * @param zeqSchema the ZeroEQ schema used to send/receive frame events.
     * @param parentWgt Parent widget.
     */
    AnimationController( Controller& controller,
                         const servus::URI& zeqSchema,
                         QWidget *parentWgt = 0 );
    ~AnimationController( );

signals:

    void newFrameReceived( ::zeq::hbp::data::Frame frame );
    void firstHeartBeatReceived();

private Q_SLOTS:

    void _connect();
    void _disconnect();
    void _onSliderMoved();
    void _togglePlayPause();
    void _onNewFrameReceived( ::zeq::hbp::data::Frame frame );
    void _onFirstHeartBeatReceived();

private:

    struct Impl;
    Impl* _impl;
};

}
#endif
