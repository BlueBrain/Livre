/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/Lib/Configuration/ApplicationParameters.h>

namespace vmml
{

std::istream& operator>>( std::istream& is, vmml::vector<3ul, float>& vec )
{
    return is >> std::skipws >> vec.x() >> vec.y() >> vec.z();
}

}

namespace livre
{

const std::string DATAFILE_PARAM = "volume";
const std::string NUMFRAMES_PARAM = "num-frames";
const std::string CAMERAPOS_PARAM = "camera-position";
const std::string SYNC_CAMERA_PARAM = "sync-camera";

ApplicationParameters::ApplicationParameters()
    : Parameters( "Application Parameters" )
    , maxFrames(-1u )
    , isResident( false )
    , cameraPosition( Vector3f( 0, 0, -2.0 ))
    , syncCamera( false )
{
    configuration_.addDescription( configGroupName_, DATAFILE_PARAM,
                                   "URI of volume data source", dataFileName );
    configuration_.addDescription( configGroupName_, NUMFRAMES_PARAM,
                                   "Maximum nuber of frames", maxFrames );
    configuration_.addDescription( configGroupName_, CAMERAPOS_PARAM,
                                   "Camera position", cameraPosition );
#ifdef LIVRE_USE_ZEQ
    configuration_.addDescription( configGroupName_, SYNC_CAMERA_PARAM,
                                   "Synchronize camera with other applications",
                                   syncCamera );
#endif
}

void ApplicationParameters::serialize( co::DataOStream &os,
                                       const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );

    os << dataFileName
       << maxFrames
       << isResident
       << cameraPosition
       << syncCamera;
}

ApplicationParameters& ApplicationParameters::operator=(
        const ApplicationParameters& parameters )
{
    if( this == &parameters )
        return *this;

    dataFileName = parameters.dataFileName;
    maxFrames = parameters.maxFrames;
    isResident = parameters.isResident;
    cameraPosition = parameters.cameraPosition;
    syncCamera = parameters.syncCamera;

    return *this;
}

void ApplicationParameters::deserialize( co::DataIStream &is,
                                         const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );

    is >> dataFileName
       >> maxFrames
       >> isResident
       >> cameraPosition
       >> syncCamera;
}

void ApplicationParameters::initialize_()
{
    configuration_.getValue( DATAFILE_PARAM, dataFileName );
    configuration_.getValue( NUMFRAMES_PARAM, maxFrames );
    configuration_.getValue( CAMERAPOS_PARAM, cameraPosition );
    configuration_.getValue( SYNC_CAMERA_PARAM, syncCamera );
}

}
