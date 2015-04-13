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

#ifndef _EventHandlerFactory_h_
#define _EventHandlerFactory_h_

#include <livre/core/types.h>

namespace livre
{

/**
 * The EventHandlerFactory class generates necessary eventhandler instances using the event id.
 */
class EventHandlerFactory
{
public:
    /**
     * Generates necessary eventhandler instances.
     * @param eventId The event id.
     * @return A new handler for the event.
     */
    virtual EventHandlerPtr create( const uint32_t eventId ) const = 0;

    virtual ~EventHandlerFactory() {}
};

}

#endif // _EventHandlerFactory_h_
