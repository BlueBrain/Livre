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

#include <livre/Lib/Configuration/RESTParameters.h>

#include <boost/algorithm/string.hpp>

namespace livre
{

const std::string HOSTNAME_PARAM = "resthost";
const std::string PORT_PARAM = "restport";
const std::string ZEQSCHEMA_PARAM = "zeqschema";
const std::string USEREST_PARAM = "userest";

RESTParameters::RESTParameters()
    : Parameters( "REST Parameters" )
    , hostName( "localhost" )
    , port( 4020 )
    , zeqSchema( "rest" )
    , useRESTConnector( false )
{
    configuration_.addDescription( configGroupName_, HOSTNAME_PARAM,
                                   "RESTConnector's http hostname",
                                   hostName );
    configuration_.addDescription( configGroupName_, PORT_PARAM,
                                   "RESTConnector's http port",
                                   port );
    configuration_.addDescription( configGroupName_, ZEQSCHEMA_PARAM,
                                   "RESTConnector's internal zeq schema",
                                   zeqSchema );
    configuration_.addDescription( configGroupName_, USEREST_PARAM,
                                   "Use RESTConnector",
                                   useRESTConnector );
}

void RESTParameters::deserialize( co::DataIStream &is, const uint64_t dirtyBits LB_UNUSED )
{
    is >> hostName
       >> port
       >> zeqSchema
       >> useRESTConnector;
}

void RESTParameters::serialize( co::DataOStream &os, const uint64_t dirtyBits LB_UNUSED )
{
    os << hostName
       << port
       << zeqSchema
       << useRESTConnector;
}

RESTParameters &RESTParameters::operator=( const RESTParameters &parameters )
{
    hostName = parameters.hostName;
    port = parameters.port;
    zeqSchema = parameters.zeqSchema;
    useRESTConnector = parameters.useRESTConnector;

    return *this;
}

void RESTParameters::initialize_()
{
    configuration_.getValue( HOSTNAME_PARAM, hostName );
    configuration_.getValue( PORT_PARAM, port );
    configuration_.getValue( ZEQSCHEMA_PARAM, zeqSchema );
    configuration_.getValue( USEREST_PARAM, useRESTConnector );
}

}
