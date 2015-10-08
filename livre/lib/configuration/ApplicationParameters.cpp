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

const std::string ANIMATION_PARAM = "animation";
const std::string ANIMATION_FOLLOW_DATA_PARAM = "animation-follow-data";
const std::string FRAMES_PARAM = "frames";
const std::string NUMFRAMES_PARAM = "num-frames";
const std::string CAMERAPOS_PARAM = "camera-position";
const std::string CAMERALOOKAT_PARAM = "camera-lookat";
const std::string SYNC_CAMERA_PARAM = "sync-camera";
const std::string ZEQSCHEMA_PARAM = "zeq-schema";

ApplicationParameters::ApplicationParameters()
    : cameraPosition( 0, 0, 1 )
    , cameraLookAt( 0, 0, 0 )
    , frames( 0, std::numeric_limits<Vector2ui::value_type>::max( ))
    , maxFrames( 0xFFFFFFFFu )
    , animation( 0 )
    , isResident( false )
    , syncCamera( false )
    , zeqSchema( "hbp://" )
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
#ifdef LIVRE_USE_ZEQ
    configuration_.addDescription( configGroupName_, SYNC_CAMERA_PARAM,
                                   "Synchronize camera with other applications",
                                   syncCamera );
    configuration_.addDescription( configGroupName_, ZEQSCHEMA_PARAM,
                                   "Zeq schema",
                                   zeqSchema );
#endif
}

ApplicationParameters& ApplicationParameters::operator = (
    const ApplicationParameters& parameters )
{
    if( this == &parameters )
        return *this;

    ClientParameters::operator = ( parameters );
    cameraPosition = parameters.cameraPosition;
    cameraLookAt = parameters.cameraLookAt;
    frames = parameters.frames;
    maxFrames = parameters.maxFrames;
    animation = parameters.animation;
    isResident = parameters.isResident;
    syncCamera = parameters.syncCamera;
    zeqSchema = parameters.zeqSchema;

    return *this;
}

void ApplicationParameters::initialize_()
{
    ClientParameters::initialize_();
    configuration_.getValue( ANIMATION_PARAM, animation );
    configuration_.getValue( FRAMES_PARAM, frames );
    configuration_.getValue( NUMFRAMES_PARAM, maxFrames );
    configuration_.getValue( CAMERAPOS_PARAM, cameraPosition );
    configuration_.getValue( CAMERALOOKAT_PARAM, cameraLookAt );
#ifdef LIVRE_USE_ZEQ
    configuration_.getValue( SYNC_CAMERA_PARAM, syncCamera );
    configuration_.getValue( ZEQSCHEMA_PARAM, zeqSchema );
#endif
    bool animationFollowData = false;
    configuration_.getValue( ANIMATION_FOLLOW_DATA_PARAM, animationFollowData );
    if( animationFollowData )
        animation = INT_MAX;
}

}
