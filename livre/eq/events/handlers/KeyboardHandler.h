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

#ifndef _KeyboardHandler_h_
#define _KeyboardHandler_h_

#include <livre/eq/events/EqEventHandler.h>

namespace livre
{

/**
 * The KeyboardHandler class handles the keyboard events for frame settings changes.
 */
class KeyboardHandler : public EqEventHandler
{
public:

    KeyboardHandler( );

    /**
     * Handles the event.
     * @param eventInfo Event information.
     * @return True if event is handled.
     */
    virtual bool operator ()( EqEventInfo& eventInfo );

private:

    bool forceUpdate_;
};

}

#endif // _KeyboardHandler_h_
