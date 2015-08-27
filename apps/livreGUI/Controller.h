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
#ifdef LIVRE_USE_ISC
#   include <isc/types.h>
#endif
namespace livre
{

/**
 * Controls communication between the GUI and the rendering application.
 * TODO: The widgets for connection can be provided by this interface.
 */
class Controller
{
public:

    Controller( );
    ~Controller( );

    /**
     * TODO: use zeq::connection::Broker to connect livre subscriber to GUI
     * @param hostname Hostname to connect.
     * @param port Port of the host to connect.
     * @return True if connection is successful.
     */
    bool connect( const std::string& hostname,
                  uint16_t port );

    /**
     * @param uri Publisher uri. If there is no publisher with the given uri
     * a new publisher is registered and returned, if there is registered
     * publisher is returned.
     * @param event the serialized event to publish
     * @return true if publish was successful
     */
    bool publish( const servus::URI& uri,
                  const zeq::Event& event );

    /**
     * @param uri Subscriber uri. If there is no subscriber with the
     * given uri a new subscriber is registered, if there is registered
     * it will be used.
     * @param event the event type of interest
     * @param func the callback function on receive of event
     * @return true if function can be registered
     */
    bool registerHandler( const servus::URI& uri,
                          const servus::uint128_t& event,
                          const zeq::EventFunc& func );

    /**
     * @param uri Subscriber uri.
     * @param event the event type of interest
     * @param func the callback function that receives the event
     * @return true if function can be unregistered
     */
    bool deregisterHandler( const servus::URI& uri,
                            const servus::uint128_t& event,
                            const zeq::EventFunc& func );

#ifdef LIVRE_USE_ISC
    /**
     * @param uri Simulator uri
     * @return If there is no simulator with the given uri a new simulator is
     * registered and returned, if there is registered simulator is returned.
     */
    isc::Simulator* getSimulator( const servus::URI& uri );
#endif

private:

    struct Impl;
    Impl* _impl;
};

}
#endif  // MASS_VOL__GUI_CONTROLLER_H
