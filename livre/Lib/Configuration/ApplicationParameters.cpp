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
#include <livre/core/mathTypes.h>

namespace vmml
{

std::istream& operator>>( std::istream& is, Vector3f& vec )
{
    return is >> std::skipws >> vec.x() >> vec.y() >> vec.z();
}

std::istream& operator>>( std::istream& is, Vector2ui& vec )
{
    return is >> std::skipws >> vec.x() >> vec.y();
}

}

namespace livre
{

const std::string ANIMATIONENABLED_PARAM = "animation";
const std::string FRAMES_PARAM = "frames";
const std::string DATAFILE_PARAM = "volume";
const std::string NUMFRAMES_PARAM = "num-frames";
const std::string CAMERAPOS_PARAM = "camera-position";
const std::string SYNC_CAMERA_PARAM = "sync-camera";

ApplicationParameters::ApplicationParameters()
    : Parameters( "Application Parameters" )
    , animationEnabled( false )
    , frames( Vector2ui( 0, std::numeric_limits< Vector2ui::value_type >::max( )))
    , maxFrames(-1u )
    , isResident( false )
    , cameraPosition( Vector3f( 0, 0, -2.0 ))
    , syncCamera( false )
{
    configuration_.addDescription( configGroupName_, DATAFILE_PARAM,
                                   "URI of volume data source", dataFileName );
    configuration_.addDescription( configGroupName_, ANIMATIONENABLED_PARAM,
                                   "Enable animation mode", animationEnabled );
    configuration_.addDescription( configGroupName_, FRAMES_PARAM,
                                   "Frames to render 'start end'", frames );
    configuration_.addDescription( configGroupName_, NUMFRAMES_PARAM,
                                   "Maximum nuber of frames to render", maxFrames );
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
       << animationEnabled
       << frames
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
    animationEnabled = parameters.animationEnabled;
    frames = parameters.frames;
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
       >> animationEnabled
       >> frames
       >> maxFrames
       >> isResident
       >> cameraPosition
       >> syncCamera;
}

void ApplicationParameters::initialize_()
{
    configuration_.getValue( DATAFILE_PARAM, dataFileName );
    configuration_.getValue( ANIMATIONENABLED_PARAM, animationEnabled );
    configuration_.getValue( FRAMES_PARAM, frames );
    configuration_.getValue( NUMFRAMES_PARAM, maxFrames );
    configuration_.getValue( CAMERAPOS_PARAM, cameraPosition );
    configuration_.getValue( SYNC_CAMERA_PARAM, syncCamera );
}

}
