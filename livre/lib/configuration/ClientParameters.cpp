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

#include <livre/lib/configuration/ClientParameters.h>

namespace livre
{
namespace
{
const std::string DATAFILE_PARAM = "volume";
const std::string COMPUTE_THREADSPERWINDOW_PARAM = "compute-threads-per-window";
const std::string UPLOAD_THREADSPERWINDOW_PARAM = "upload-threads-per-window";
}

ClientParameters::ClientParameters()
    : Parameters( "Application Parameters" )
    , computeThreadsPerWindow( 4 )
    , uploadThreadsPerWindow( 4 )
{
    configuration_.addDescription( configGroupName_, DATAFILE_PARAM,
                                   "URI of volume data source", dataFileName );
    configuration_.addDescription( configGroupName_, COMPUTE_THREADSPERWINDOW_PARAM,
                                   "Compute threads per window", computeThreadsPerWindow );
    configuration_.addDescription( configGroupName_, UPLOAD_THREADSPERWINDOW_PARAM,
                                   "Upload threads per window", uploadThreadsPerWindow );
}

void ClientParameters::serialize( co::DataOStream &os,
                                  const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );
    os << dataFileName
       << computeThreadsPerWindow
       << uploadThreadsPerWindow;
}

ClientParameters& ClientParameters::operator = (
        const ClientParameters& parameters )
{
    if( this == &parameters )
        return *this;

    dataFileName = parameters.dataFileName;
    computeThreadsPerWindow = parameters.computeThreadsPerWindow;
    uploadThreadsPerWindow = parameters.uploadThreadsPerWindow;
    return *this;
}

void ClientParameters::deserialize( co::DataIStream &is,
                                    const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );
    is >> dataFileName
       >> computeThreadsPerWindow
       >> uploadThreadsPerWindow;
}

void ClientParameters::initialize_()
{
    configuration_.getValue( DATAFILE_PARAM, dataFileName );
    configuration_.getValue( COMPUTE_THREADSPERWINDOW_PARAM, computeThreadsPerWindow );
    configuration_.getValue( UPLOAD_THREADSPERWINDOW_PARAM, uploadThreadsPerWindow );
}
}
