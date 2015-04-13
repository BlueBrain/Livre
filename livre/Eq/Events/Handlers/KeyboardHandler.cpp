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

#include <livre/Eq/Config.h>
#include <livre/Eq/FrameData.h>
#include <livre/Eq/Settings/CameraSettings.h>
#include <livre/Eq/Settings/FrameSettings.h>
#include <livre/Eq/Settings/RenderSettings.h>

#include <livre/Eq/Events/EqEventInfo.h>
#include <livre/Eq/Events/Handlers/KeyboardHandler.h>

#include <eq/client/configEvent.h>

namespace livre
{

KeyboardHandler::KeyboardHandler( )
   : forceUpdate_( false )
{
}

bool KeyboardHandler::operator ()( EqEventInfo& eqEventInfo )
{
    Config* config = eqEventInfo.config;
    const eq::KeyEvent& event = eqEventInfo.configEvent->data.keyPress;

    RenderSettingsPtr renderSettings = config->getFrameData().getRenderSettings();
    FrameSettingsPtr frameSettings = config->getFrameData().getFrameSettings();
    CameraSettingsPtr cameraSettings = config->getFrameData().getCameraSettings();

    if( event.key >= '1' && event.key <= '9' )
    {
        renderSettings->setMaxTreeDepth( 1 + event.key-'1' );
        return true;
    }

    switch( event.key )
    {
        case '+':
        case '=':
            renderSettings->setMaxTreeDepth( renderSettings->getMaxTreeDepth() + 1 );
            return true;

        case '-':
        case '_':
            renderSettings->setMaxTreeDepth( renderSettings->getMaxTreeDepth() - 1 );
            return true;

        case eq::KC_F1:
        case 'h':
        case 'H':
            frameSettings->toggleHelp();
            return true;

        case 'r':
        case 'R':
            frameSettings->toggleRecording();
            return true;

        case ' ':
            forceUpdate_ = true;
            config->resetCamera( );
            return true;

        case 's':
        case 'S':
            frameSettings->toggleStatistics();
            return true;

        case 'l':
            config->switchLayout_( 1 );
            return true;

        case 'L':
            config->switchLayout_( -1 );
            return true;

        case 'm':
        case 'M':
            cameraSettings->togglePilotMode();
            return true;

        case 'p':
        case 'P':
            frameSettings->makeScreenshot();
            return true;

        case 'c':
        case 'C':
        {
            return  config->switchCanvas_();
        }

        case 'v':
        case 'V':
        {
            return config->switchView_();
        }

        default:
            break;
    }
    return false;
}


}
