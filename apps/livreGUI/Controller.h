/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                     Maxim Makhinya <maxmah@gmail.com>
 *                     David Steiner  <steiner@ifi.uzh.ch>
 *                     Fatih Erol
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

#ifndef _Controller_h_
#define _Controller_h_

#include <zeq/types.h>

namespace livre
{

/**
 * Provides continuous receive on the subscriber for registered events and posts
 * requests for registered events on the heartbeat event of Livre. This allows
 * for initializing the GUI once the first livre instance is connected.
 */
class Controller
{
public:
    Controller();
    ~Controller();

    /**
     * @param event the serialized event to publish
     * @return true if the publication of the event was successful, false
     *         otherwise
     */
    bool publish( const zeq::Event& event );

    /**
     * Register the handler for the given event and publish requests for that
     * event as well.
     *
     * @param event the event type of interest
     * @param func the callback function on receive of event
     * @return true if the function was successfully registered
     */
    bool registerHandler( const zeq::uint128_t& event,
                          const zeq::EventFunc& func );

    /**
     * @param event the event type of interest
     * @return true if function was successfully deregistered
     */
    bool deregisterHandler( const zeq::uint128_t& event );

private:
    class Impl;
    std::unique_ptr< Impl > _impl;
};

}
#endif  // _Controller_h_
