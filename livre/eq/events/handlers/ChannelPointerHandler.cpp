/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#include <eq/eq.h>

#include <livre/eq/Config.h>
#include <livre/eq/events/EqEventInfo.h>
#include <livre/eq/events/handlers/ChannelPointerHandler.h>
#include <livre/eq/settings/EqCameraSettings.h>
#include <livre/eq/settings/EqFrameSettings.h>
#include <livre/eq/FrameData.h>

#include <eq/fabric/pointerEvent.h>

#define SPINX_START 5
#define SPINY_START 5
#define ADVANCE_START 0
#define ROTATE_AND_ZOOM_SPEED 0.005f
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
    const eq::PointerEvent& event =
            static_cast< const eq::PointerEvent& >( eqEventInfo.event );
    Config* config = eqEventInfo.config;

    CameraSettings& cameraSettings =
            config->getFrameData().getCameraSettings();

    switch( eqEventInfo.type )
    {
        case eq::EVENT_CHANNEL_POINTER_BUTTON_PRESS:
        {
            const eq::uint128_t& viewID = event.context.view.identifier;
            return config->switchToViewCanvas( viewID );
        }

        case eq::EVENT_CHANNEL_POINTER_MOTION:
        {
            switch( event.buttons )
            {
              case eq::PTR_BUTTON1:
                  spinX_ = 0;
                  spinY_ = 0;

                  cameraSettings.spinModel(
                      -ROTATE_AND_ZOOM_SPEED * event.dy,
                      -ROTATE_AND_ZOOM_SPEED * event.dx );
                  return true;

              case eq::PTR_BUTTON2:
                  advance_ = -event.dy;
                  cameraSettings.moveCamera( 0.f, 0.f,
                                              ROTATE_AND_ZOOM_SPEED * advance_ );
                  return true;

              case eq::PTR_BUTTON3:
                  cameraSettings.moveCamera( PAN_SPEED * event.dx,
                                             -PAN_SPEED * event.dy,
                                             0.f );
                  return true;
            }
            break;
        }

        case eq::EVENT_CHANNEL_POINTER_WHEEL:
        {
            cameraSettings.moveCamera( -ADVANCE_SPEED * event.xAxis,
                                       0.f,
                                       ADVANCE_SPEED * event.yAxis );
            return true;
        }
        default:
            return false;

    }

    return false;
}

}
