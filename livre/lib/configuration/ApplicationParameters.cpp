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
const std::string ANIMATION_FPS_PARAM = "animation-fps";
const std::string ANIMATION_FOLLOW_DATA_PARAM = "animation-follow-data";
const std::string FRAMES_PARAM = "frames";
const std::string NUMFRAMES_PARAM = "num-frames";
const std::string CAMERAPOS_PARAM = "camera-position";
const std::string CAMERALOOKAT_PARAM = "camera-lookat";
const std::string SYNC_CAMERA_PARAM = "sync-camera";
const std::string DATAFILE_PARAM = "volume";
const std::string COLORMAP_PARAM = "colormap";

ApplicationParameters::ApplicationParameters()
    : Parameters( "Application Parameters" )
    , cameraPosition( 0, 0, 1.5 )
    , cameraLookAt( 0, 0, 0 )
    , frames( FULL_FRAME_RANGE )
    , maxFrames( std::numeric_limits< uint32_t >::max( ))
    , animation( 0 )
    , animationFPS( 0 )
    , isResident( false )
{
    _configuration.addDescription( configGroupName_, ANIMATION_PARAM,
                                   "Enable animation mode (optional frame delta for animation "
                                   "speed, use --animation=-<int> for reverse animation)",
                                   animation, 1 );
    _configuration.addDescription( configGroupName_, ANIMATION_FPS_PARAM,
                                   "Animation frames per second. By default (value of 0), it will "
                                   "request a new frame as soon as the previous one is done",
                                   animationFPS );
    _configuration.addDescription( configGroupName_, ANIMATION_FOLLOW_DATA_PARAM,
                                   "Enable animation and follow volume data stream (overrides "
                                   "--animation=value)", false );
    _configuration.addDescription( configGroupName_, FRAMES_PARAM,
                                   "Frames to render [start end)", frames );
    _configuration.addDescription( configGroupName_, NUMFRAMES_PARAM,
                                   "Maximum nuber of frames to render", maxFrames );
    _configuration.addDescription( configGroupName_, CAMERAPOS_PARAM,
                                   "Camera position", cameraPosition );
    _configuration.addDescription( configGroupName_, CAMERALOOKAT_PARAM,
                                   "Camera orientation", cameraLookAt );
    _configuration.addDescription( configGroupName_, DATAFILE_PARAM,
                                   "URI of volume data source", dataFileName );
    _configuration.addDescription( configGroupName_, COLORMAP_PARAM,
                                   "Color map file (*.lba, *.lbb)",
                                   colorMap );
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
    animationFPS = parameters.animationFPS;
    isResident = parameters.isResident;
    dataFileName = parameters.dataFileName;
    colorMap = parameters.colorMap;

    return *this;
}

void ApplicationParameters::_initialize()
{
    animation = _configuration.getValue( ANIMATION_PARAM, animation );
    animationFPS = _configuration.getValue( ANIMATION_FPS_PARAM, animationFPS );
    frames = _configuration.getValue( FRAMES_PARAM, frames );
    maxFrames = _configuration.getValue( NUMFRAMES_PARAM, maxFrames );
    cameraPosition = _configuration.getValue( CAMERAPOS_PARAM, cameraPosition );
    cameraLookAt = _configuration.getValue( CAMERALOOKAT_PARAM, cameraLookAt );
    dataFileName = _configuration.getValue( DATAFILE_PARAM, dataFileName );
    colorMap = _configuration.getValue( COLORMAP_PARAM, colorMap );
    bool animationFollowData = false;
    animationFollowData = _configuration.getValue( ANIMATION_FOLLOW_DATA_PARAM,
                                                   animationFollowData );
    if( animationFollowData )
        animation = LATEST_FRAME;
}

}
