/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#include <livre/lib/configuration/ApplicationParameters.h>

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

const std::string ANIMATION_PARAM = "animation";
const std::string ANIMATION_FOLLOW_DATA_PARAM = "animation-follow-data";
const std::string FRAMES_PARAM = "frames";
const std::string NUMFRAMES_PARAM = "num-frames";
const std::string CAMERAPOS_PARAM = "camera-position";
const std::string CAMERALOOKAT_PARAM = "camera-lookat";
const std::string SYNC_CAMERA_PARAM = "sync-camera";
const std::string DATAFILE_PARAM = "volume";
const std::string TRANSFERFUNCTION_PARAM = "transfer-function";

ApplicationParameters::ApplicationParameters()
    : Parameters( "Application Parameters" )
    , cameraPosition( 0, 0, 1 )
    , cameraLookAt( 0, 0, 0 )
    , frames( FULL_FRAME_RANGE )
    , maxFrames( -1u )
    , animation( 0 )
    , isResident( false )
{
    configuration_.addDescription( configGroupName_, ANIMATION_PARAM,
                                   "Enable animation mode (optional frame delta for animation speed, use --animation=-<int> for reverse animation)", animation, 1 );
    configuration_.addDescription( configGroupName_, ANIMATION_FOLLOW_DATA_PARAM,
                                   "Enable animation and follow volume data stream (overrides --animation=value)", false );
    configuration_.addDescription( configGroupName_, FRAMES_PARAM,
                                   "Frames to render [start end)", frames );
    configuration_.addDescription( configGroupName_, NUMFRAMES_PARAM,
                                   "Maximum nuber of frames to render", maxFrames );
    configuration_.addDescription( configGroupName_, CAMERAPOS_PARAM,
                                   "Camera position", cameraPosition );
    configuration_.addDescription( configGroupName_, CAMERALOOKAT_PARAM,
                                   "Camera orientation", cameraLookAt );
    configuration_.addDescription( configGroupName_, DATAFILE_PARAM,
                                   "URI of volume data source", dataFileName );
    configuration_.addDescription( configGroupName_, TRANSFERFUNCTION_PARAM,
                                ".1dt transfer function file (from ImageVis3D)",
                                   transferFunction );
}

ApplicationParameters& ApplicationParameters::operator = (
    const ApplicationParameters& parameters )
{
    if( this == &parameters )
        return *this;

    cameraPosition = parameters.cameraPosition;
    cameraLookAt = parameters.cameraLookAt;
    frames = parameters.frames;
    maxFrames = parameters.maxFrames;
    animation = parameters.animation;
    isResident = parameters.isResident;
    dataFileName = parameters.dataFileName;
    transferFunction = parameters.transferFunction;

    return *this;
}

void ApplicationParameters::initialize_()
{
    animation = configuration_.getValue( ANIMATION_PARAM, animation );
    frames = configuration_.getValue( FRAMES_PARAM, frames );
    maxFrames = configuration_.getValue( NUMFRAMES_PARAM, maxFrames );
    cameraPosition = configuration_.getValue( CAMERAPOS_PARAM, cameraPosition );
    cameraLookAt = configuration_.getValue( CAMERALOOKAT_PARAM, cameraLookAt );
    dataFileName = configuration_.getValue( DATAFILE_PARAM, dataFileName );
    transferFunction = configuration_.getValue( TRANSFERFUNCTION_PARAM,
                                                transferFunction );
    bool animationFollowData = false;
    animationFollowData = configuration_.getValue( ANIMATION_FOLLOW_DATA_PARAM,
                                                   animationFollowData );
    if( animationFollowData )
        animation = LATEST_FRAME;
}

}
