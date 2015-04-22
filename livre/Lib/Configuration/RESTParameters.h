/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                     Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#ifndef _RESTParameters_h_
#define _RESTParameters_h_

#include <livre/core/mathTypes.h>
#include <livre/core/Configuration/Parameters.h>

namespace livre
{

/**
 * The RESTParameters struct keeps the parameters for RESTConnector.
 */
struct RESTParameters : public Parameters
{
    RESTParameters( );

    std::string hostName; //<! hostname for RESTConnector.
    uint16_t port; //<! port for RESTConnector.
    std::string zeqSchema; //<! zeq schema for RESTConncetor and Livre
    bool useRESTConnector;

    /**
     * De-serializes the object from input stream.
     * @param dirtyBits The bits for elements can be marked
     * @param is Input stream.
     */
    virtual void deserialize( co::DataIStream& is, const uint64_t dirtyBits );

    /**
     * Serializes the object to output stream.
     * @param dirtyBits The bits for elements can be marked
     * @param os Output stream.
     */
    virtual void serialize( co::DataOStream& os, const uint64_t dirtyBits );

    /**
     * @param parameters The source parameters.
     */
    RESTParameters& operator=( const RESTParameters& parameters );

protected:

    virtual void initialize_( );
};

}

#endif // _RESTParameters_h_
