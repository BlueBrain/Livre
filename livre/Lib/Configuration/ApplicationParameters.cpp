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

const char *LOGFILE_PARAM = "logfile";
const char *DATAFILE_PARAM = "volume";
const char *NUMFRAMES_PARAM = "numframes";
const char *CAMERAPOS_PARAM = "camerapos";
const char *GUI_ENABLED_PARAM = "enablegui";

ApplicationParameters::ApplicationParameters()
    : Parameters( "Application Parameters" ),
      animationEnabled( false ),
      captureEnabled( false ),
      debugWindowEnabled( false ),
      maxFrames(-1u ),
      isResident( false ),
      cameraPosition( Vector3f( 0, 0, -2.0 ))
{
    configuration_.addDescription< std::string >( configGroupName_,
                                                  LOGFILE_PARAM, "Renderlog", "" );
    configuration_.addDescription< std::string >( configGroupName_,
                                                  DATAFILE_PARAM, dataFileName );
    configuration_.addDescription( configGroupName_,
                                   NUMFRAMES_PARAM,
                                   "Maximum nuber of frames", maxFrames );
    configuration_.addDescription( configGroupName_,
                                   CAMERAPOS_PARAM,
                                   "Camera position", cameraPosition );
}

void ApplicationParameters::serialize( co::DataOStream &os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );

    os << logFileName
       << animationPathFileName
       << animationEnabled
       << dataFileName
       << captureEnabled
       << captureName
       << capturePath
       << debugWindowEnabled
       << maxFrames
       << isResident
       << cameraPosition;
}

ApplicationParameters& ApplicationParameters::operator=(
        const ApplicationParameters& parameters )
{
    if( this == &parameters )
        return *this;

    logFileName = parameters.logFileName;
    animationPathFileName = parameters.animationPathFileName;
    animationEnabled = parameters.animationEnabled;
    dataFileName = parameters.dataFileName;
    captureEnabled = parameters.captureEnabled;
    captureName = parameters.captureName;
    capturePath = parameters.capturePath;
    debugWindowEnabled = parameters.debugWindowEnabled;
    maxFrames = parameters.maxFrames;
    isResident = parameters.isResident;
    cameraPosition = parameters.cameraPosition;

    return *this;
}

void ApplicationParameters::deserialize( co::DataIStream &is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );

    is >> logFileName
       >> animationPathFileName
       >> animationEnabled
       >> dataFileName
       >> captureEnabled
       >> captureName
       >> capturePath
       >> debugWindowEnabled
       >> maxFrames
       >> isResident
       >> cameraPosition;
}

void ApplicationParameters::initializeParameters_()
{
    configuration_.getValue( LOGFILE_PARAM, logFileName );
    configuration_.getValue( DATAFILE_PARAM, dataFileName );
    configuration_.getValue( NUMFRAMES_PARAM, maxFrames );
    configuration_.getValue( CAMERAPOS_PARAM, cameraPosition );
}

}
