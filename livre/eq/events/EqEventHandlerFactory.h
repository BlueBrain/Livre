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

#include <livre/core/events/EventHandlerFactory.h>

#include <livre/eq/types.h>

namespace livre
{

/**
 * The EqEventHandlerFactory class generates \see EqEventHandler instances for handling Equalizer events
 */
class EqEventHandlerFactory : public EventHandlerFactory
{

public:

    EqEventHandlerFactory( );

    /**
     * \see livre::EventHandlerFactory::create()
     */
    virtual EventHandlerPtr create( const uint32_t eventId ) const;

};

}
