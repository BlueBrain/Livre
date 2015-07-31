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

#include <livre/core/events/EventMapper.h>
#include <livre/core/events/EventHandler.h>
#include <livre/core/events/EventHandlerFactory.h>

namespace livre
{

EventMapper::EventMapper( EventHandlerFactoryPtr eventHandlerFactoryPtr )
    : eventHandlerFactoryPtr_( eventHandlerFactoryPtr )
{

}

bool EventMapper::registerEvent( const uint32_t eventId, EventHandlerPtr eventHandler )
{
    EventHandlerPtrMap::iterator it = eventHandlerPtrMap_.find( eventId );
    if( it != eventHandlerPtrMap_.end() )
    {
        return false;
    }

    EventHandlerPtr handler = eventHandler;
    if( !handler )
    {
        handler = eventHandlerFactoryPtr_->create( eventId );
    }

    eventHandlerPtrMap_[ eventId ] = handler;
    return true;
}

bool EventMapper::unregisterEvent(const uint32_t eventId)
{
    EventHandlerPtrMap::iterator it = eventHandlerPtrMap_.find( eventId );
    if( it == eventHandlerPtrMap_.end() )
    {
        return false;
    }

    eventHandlerPtrMap_.erase( eventId );
    return true;
}

bool EventMapper::handleEvent( const uint32_t eventId, EventInfo& eventInfo ) const
{
    EventHandlerPtrMap::const_iterator it = eventHandlerPtrMap_.find( eventId );
    if( it == eventHandlerPtrMap_.end() )
    {
        return false;
    }

    return (*it->second)( eventInfo );
}

EventHandlerPtr EventMapper::getEventHandler(const uint32_t eventId) const
{
    EventHandlerPtrMap::const_iterator it = eventHandlerPtrMap_.find( eventId );
    if( it == eventHandlerPtrMap_.end() )
    {
        return EventHandlerPtr();
    }
    return it->second;
}




}
