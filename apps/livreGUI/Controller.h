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

#include <livre/core/Render/TransferFunction1D.h>
#include <zeq/zeq.h>

namespace livre
{

/**
 * Controls communication between the GUI and the rendering application.
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
    bool connect( const std::string &hostname, const uint16_t port );

    /**
     * Updates the transfer function.
     * @return True if connection was established.
     */
    void publishTransferFunction( );

    /**
     * @return The transfer function.
     */
    TransferFunction1DfPtr getTransferFunction() const;

private:
    TransferFunction1DfPtr _transferFunction;
    zeq::Publisher _publisher;
};

}


#endif  // MASS_VOL__GUI_CONTROLLER_H
