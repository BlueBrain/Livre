/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Maxim Makhinya <maxmah@gmail.com>
 *                          David Steiner  <steiner@ifi.uzh.ch>
 *                          Fatih Erol
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

#include <zerobuf/types.h>

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
     * @param zerobuf the ZeroBuf object to publish
     * @return true if the publication of the object was successful, false
     *         otherwise
     */
    bool publish(const ::zerobuf::Zerobuf& zerobuf);

    /**
     * Subscribe the given ZeroBuf object and publish a request for a first-time
     * init of that object.
     *
     * @param zerobuf the ZeroBuf object to subscribe for updates
     * @return true if the object was successfully subscribed
     */
    bool subscribe(::zerobuf::Zerobuf& zerobuf);

    /**
     * Unsubscribe the given ZeroBuf object from receival of updates.
     *
     * @param zerobuf the ZeroBuf object to unsubscribe
     * @return true if the objec twas successfully unsubscribed
     */
    bool unsubscribe(const ::zerobuf::Zerobuf& zerobuf);

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};
}
#endif // _Controller_h_
