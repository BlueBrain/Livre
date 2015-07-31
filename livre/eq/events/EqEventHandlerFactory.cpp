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

#include <livre/eq/events/Events.h>
#include <livre/eq/events/EqEventHandlerFactory.h>
#include <livre/eq/events/handlers/ChannelPointerHandler.h>
#include <livre/eq/events/handlers/KeyboardHandler.h>

namespace livre
{

EqEventHandlerFactory::EqEventHandlerFactory( )
{}

EventHandlerPtr EqEventHandlerFactory::create( const uint32_t eventId ) const
{
    EventHandlerPtr eventHandler;

    switch( eventId )
    {
    case EVENT_CHANNEL_POINTER:
        eventHandler.reset( new ChannelPointerHandler( ) );
        break;
    case EVENT_KEYBOARD:
        eventHandler.reset( new KeyboardHandler( ) );
        break;
    }

    return eventHandler;
}



}
