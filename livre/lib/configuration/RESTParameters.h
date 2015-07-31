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

#include <livre/lib/api.h>
#include <livre/core/mathTypes.h>
#include <livre/core/configuration/Parameters.h>

namespace livre
{

/**
 * The RESTParameters struct keeps the parameters for RESTBridge.
 */
struct RESTParameters : public Parameters
{
    LIVRE_API RESTParameters( );

    std::string hostName; //<! hostname for RESTBridge.
    uint16_t port; //<! port for RESTBridge.
    std::string zeqSchema; //<! zeq schema for RESTBridge and Livre
    bool useRESTBridge;

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
    LIVRE_API RESTParameters& operator=( const RESTParameters& parameters );

protected:

    virtual void initialize_( );
};

}

#endif // _RESTParameters_h_
