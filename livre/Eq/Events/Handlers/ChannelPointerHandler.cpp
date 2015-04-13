/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <eq/eq.h>

#include <livre/Eq/Config.h>
#include <livre/Eq/Events/EqEventInfo.h>
#include <livre/Eq/Events/Handlers/ChannelPointerHandler.h>
#include <livre/Eq/Settings/CameraSettings.h>
#include <livre/Eq/Settings/FrameSettings.h>
#include <livre/Eq/FrameData.h>

#define SPINX_START 5
#define SPINY_START 5
#define ADVANCE_START 0
#define ROTATE_SPEED 0.005f
#define PAN_SPEED 0.0005f
#define ADVANCE_SPEED 0.05f

namespace livre
{

ChannelPointerHandler::ChannelPointerHandler( )
   : spinX_( SPINX_START ),
     spinY_( SPINY_START ),
     advance_( ADVANCE_START )
{
}

bool ChannelPointerHandler::operator ()( EqEventInfo& eqEventInfo )
{
    const eq::ConfigEvent* event = eqEventInfo.configEvent;
    Config* config = eqEventInfo.config;

    CameraSettingsPtr cameraSettings = config->getFrameData().getCameraSettings();

    switch( event->data.type )
    {
        case eq::Event::CHANNEL_POINTER_BUTTON_PRESS:
        {
            const eq::uint128_t& viewID = event->data.context.view.identifier;
            return config->switchToViewCanvas_( viewID );
        }

        case eq::Event::CHANNEL_POINTER_BUTTON_RELEASE:
        {
            const eq::PointerEvent& releaseEvent = event->data.pointerButtonRelease;
            if( releaseEvent.buttons == eq::PTR_BUTTON_NONE)
            {
                if( releaseEvent.button == eq::PTR_BUTTON1 )
                {
                    spinX_ = releaseEvent.dy;
                    spinY_ = releaseEvent.dx;
                    return true;
                }
                if( releaseEvent.button == eq::PTR_BUTTON2 )
                {
                    advance_ = -releaseEvent.dy;
                    return true;
                }
            }
            break;
        }

        case eq::Event::CHANNEL_POINTER_MOTION:
        {
            switch( event->data.pointerMotion.buttons )
            {
              case eq::PTR_BUTTON1:
                  spinX_ = 0;
                  spinY_ = 0;

                  if( cameraSettings->getPilotMode())
                      cameraSettings->spinCamera(
                          -ROTATE_SPEED * event->data.pointerMotion.dy,
                          -ROTATE_SPEED * event->data.pointerMotion.dx );
                  else
                      cameraSettings->spinModel(
                          -ROTATE_SPEED * event->data.pointerMotion.dy,
                          -ROTATE_SPEED * event->data.pointerMotion.dx, 0.f );
                  return true;

              case eq::PTR_BUTTON2:
                  advance_ = -event->data.pointerMotion.dy;
                  cameraSettings->moveCamera( 0.f, 0.f, ADVANCE_SPEED * advance_ );
                  return true;

              case eq::PTR_BUTTON3:
                  cameraSettings->moveCamera(  PAN_SPEED * event->data.pointerMotion.dx,
                                              -PAN_SPEED * event->data.pointerMotion.dy,
                                                0.f );
                  return true;
            }
            break;
        }

        case eq::Event::CHANNEL_POINTER_WHEEL:
        {
            cameraSettings->moveCamera( -ADVANCE_SPEED * event->data.pointerWheel.xAxis,
                                         0.f,
                                         ADVANCE_SPEED * event->data.pointerWheel.yAxis );
            break;
        }
    }

    return true;
}

}
