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

#include <livre/eq/Config.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/settings/FrameSettings.h>
#include <livre/eq/settings/RenderSettings.h>

#include <livre/eq/events/EqEventInfo.h>
#include <livre/eq/events/handlers/KeyboardHandler.h>

#include <eq/fabric/keyEvent.h>

namespace livre
{

KeyboardHandler::KeyboardHandler( )
   : forceUpdate_( false )
{
}

bool KeyboardHandler::operator ()( EqEventInfo& eqEventInfo )
{
    Config* config = eqEventInfo.config;
    const eq::KeyEvent& event =
            static_cast< const eq::KeyEvent& >( eqEventInfo.event );

    RenderSettings& renderSettings = config->getFrameData().getRenderSettings();
    FrameSettings& frameSettings = config->getFrameData().getFrameSettings();

    if( event.key >= '1' && event.key <= '9' )
    {
        renderSettings.setMaxTreeDepth( 1 + event.key-'1' );
        return true;
    }

    switch( event.key )
    {
        case '+':
        case '=':
            renderSettings.setMaxTreeDepth( renderSettings.getMaxTreeDepth()+1);
            return true;

        case '-':
        case '_':
            renderSettings.setMaxTreeDepth( renderSettings.getMaxTreeDepth()-1);
            return true;

        case 'i':
        case 'I':
            frameSettings.toggleInfo();
            return true;

        case ' ':
            forceUpdate_ = true;
            config->resetCamera( );
            return true;

        case 's':
        case 'S':
            frameSettings.toggleStatistics();
            return true;

        case 'l':
            config->switchLayout( 1 );
            return true;

        case 'L':
            config->switchLayout( -1 );
            return true;

        case 'p':
        case 'P':
            frameSettings.makeScreenshot();
            return true;

        case 'c':
        case 'C':
        {
            return config->switchCanvas();
        }

        case 'v':
        case 'V':
        {
            return config->switchView();
        }

        default:
            break;
    }
    return false;
}


}
