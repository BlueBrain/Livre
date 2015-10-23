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

#include <livre/core/render/TransferFunction1D.h>
#include <zeq/types.h>

namespace livre
{

/**
 * Allows having a set of publishers on different URIs, as well as subscribers
 * that can register different functions for different event types.
 *
 * Controls communication between the GUI and the rendering application.
 * TODO: The widgets for connection can be provided by this interface.
 */
class Controller
{
public:
    Controller();
    ~Controller();

    /**
     * TODO: use zeq::connection::Broker to connect livre subscriber to GUI
     * @param hostname the hostname to connect to
     * @param port the port of the host to connect to
     * @return True if the connection was successful, false otherwise
     */
    bool connect( const std::string& hostname,
                  uint16_t port );

    /**
     * @param uri the publisher URI. If there is no publisher created on the
     * given URI then add a new one and use it to publish the specified event.
     * If a publisher object already exists, use it
     * @param event the serialized event to publish
     * @return true if the publication of the event was successful, false
     * otherwise
     */
    bool publish( const servus::URI& uri,
                  const zeq::Event& event );

    /**
     * @param uri the subscriber URI. If there is no subscriber created on the
     * given URI then add a new one use it to register a function to the
     * specified event. If a subscriber object already exists, use it.
     * @param event the event type of interest
     * @param func the callback function on receive of event
     * @return true if the function was successfully registered
     */
    bool registerHandler( const servus::URI& uri,
                          const servus::uint128_t& event,
                          const zeq::EventFunc& func );

    /**
     * @param uri the subscriber URI
     * @param event the event type of interest
     * @param func the callback function that receives the event
     * @return true if function was successfully deregistered
     */
    bool deregisterHandler( const servus::URI& uri,
                            const servus::uint128_t& event,
                            const zeq::EventFunc& func );

private:
    class Impl;
    std::unique_ptr< Impl > _impl;
};

}
#endif  // _Controller_h_
