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

#ifndef _EventMapper_h_
#define _EventMapper_h_

#include <livre/core/types.h>

namespace livre
{
/**
 * The EventMapper maps the events to handlers. It is responsible generating/removing the necessary handlers.
 */
class EventMapper
{
public:

    /**
     * @param eventHandlerFactoryPtr The factory for events.
     */
    EventMapper( EventHandlerFactoryPtr eventHandlerFactoryPtr );

    /**
     * Registers an event.
     * @param eventId Event id.
     * @param eventHandler The event handler to register if another event handler then the factory one is being used.
     * @return True if same event is not registered twice.
     */
    bool registerEvent( const uint32_t eventId, EventHandlerPtr eventHandler = EventHandlerPtr() );

    /**
     * Unregisters event.
     * @param eventId Event id.
     * @return True if event can be unregistered.
     */
    bool unregisterEvent( const uint32_t eventId );

    /**
     * Handles the event with the given id.
     * @param eventId Event id.
     * @param eventInfo The event information. \see livre::EventInfo
     * @return True when event is handled.
     */
    bool handleEvent( const uint32_t eventId, EventInfo& eventInfo ) const;

    /**
     * Gets the event from the mapper.
     * @param eventId Event id.
     * @return The event, if it finds the event, otherwise empty.
     */
    EventHandlerPtr getEventHandler( const uint32_t eventId ) const;

private:

    EventHandlerFactoryPtr eventHandlerFactoryPtr_;
    EventHandlerPtrMap eventHandlerPtrMap_;
};

}

#endif // EventMapper
