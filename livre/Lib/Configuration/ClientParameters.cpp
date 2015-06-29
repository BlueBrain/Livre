/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Stefan.Eilemann@epfl.ch
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

#include <livre/Lib/Configuration/ClientParameters.h>

namespace livre
{
namespace
{
const std::string DATAFILE_PARAM = "volume";
}

ClientParameters::ClientParameters()
    : Parameters( "Application Parameters" )
{
    configuration_.addDescription( configGroupName_, DATAFILE_PARAM,
                                   "URI of volume data source", dataFileName );
}

void ClientParameters::serialize( co::DataOStream &os,
                                       const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );
    os << dataFileName;
}

ClientParameters& ClientParameters::operator = (
        const ClientParameters& parameters )
{
    if( this != &parameters )
        dataFileName = parameters.dataFileName;
    return *this;
}

void ClientParameters::deserialize( co::DataIStream &is,
                                    const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );
    is >> dataFileName;
}

void ClientParameters::initialize_()
{
    configuration_.getValue( DATAFILE_PARAM, dataFileName );
}
}
