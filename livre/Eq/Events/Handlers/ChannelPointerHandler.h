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

#ifndef _ChannelPointerHandler_h_
#define _ChannelPointerHandler_h_

#include <livre/Eq/Events/EqEventHandler.h>

namespace livre
{

/**
 * The ChannelPointerHandler class handles the pointer events.
 */
class ChannelPointerHandler : public EqEventHandler
{
public:

    ChannelPointerHandler( );

    /**
     * Handles the event.
     * @param eqEventInfo Event information.
     * @return True if event is handled.
     */
    virtual bool operator ()( EqEventInfo& eqEventInfo );

private:

    int32_t spinX_;
    int32_t spinY_;
    int32_t advance_;
};

}

#endif // ChannelPointerHandler
