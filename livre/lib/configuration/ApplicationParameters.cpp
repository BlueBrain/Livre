/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
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

#include "ApplicationParameters.h"

#include <livre/data/DataSource.h>
#include <lunchbox/term.h>

namespace vmml
{
std::istream& operator>>(std::istream& is, Vector3f& vec)
{
    return is >> std::skipws >> vec.x() >> vec.y() >> vec.z();
}

std::istream& operator>>(std::istream& is, Vector2ui& vec)
{
    return is >> std::skipws >> vec.x() >> vec.y();
}
}

namespace
{
const char ANIMATION_PARAM[] = "animation";
const char ANIMATION_FPS_PARAM[] = "animation-fps";
const char ANIMATION_FOLLOW_DATA_PARAM[] = "animation-follow-data";
const char FRAMES_PARAM[] = "frames";
const char NUMFRAMES_PARAM[] = "num-frames";
const char CAMERAPOS_PARAM[] = "camera-position";
const char CAMERALOOKAT_PARAM[] = "camera-lookat";
const char DATAFILE_PARAM[] = "volume";
const char TRANSFERFUNCTION_PARAM[] = "transfer-function";
}

namespace livre
{
ApplicationParameters::ApplicationParameters()
    : cameraPosition(0, 0, 1.5)
    , cameraLookAt(0, 0, 0)
    , frames(FULL_FRAME_RANGE)
    , maxFrames(std::numeric_limits<uint32_t>::max())
    , animation(0)
    , animationFPS(0)
{
}
ApplicationParameters::ApplicationParameters(const int32_t argc,
                                             const char* const argv[])
    : ApplicationParameters()
{
    const auto& options = _getOptions();
    variables_map vm;
    try
    {
        boost::program_options::store(
            boost::program_options::command_line_parser(argc, argv)
                .options(options)
                .allow_unregistered()
                .run(),
            vm);
    }
    catch (const boost::program_options::error& exception)
    {
        LBTHROW(std::runtime_error(
            std::string("Error parsing application command line arguments") +
            exception.what()));
    }

    animation = vm[ANIMATION_PARAM].as<int32_t>();
    animationFPS = vm[ANIMATION_FPS_PARAM].as<uint32_t>();
    frames = vm[FRAMES_PARAM].as<Vector2ui>();
    maxFrames = vm[NUMFRAMES_PARAM].as<uint32_t>();
    cameraPosition = vm[CAMERAPOS_PARAM].as<Vector3f>();
    cameraLookAt = vm[CAMERALOOKAT_PARAM].as<Vector3f>();
    dataFileName = vm[DATAFILE_PARAM].as<std::string>();
    transferFunction = vm[TRANSFERFUNCTION_PARAM].as<std::string>();
    if (vm[ANIMATION_FOLLOW_DATA_PARAM].as<bool>())
        animation = LATEST_FRAME;
}

options_description ApplicationParameters::_getOptions() const
{
    options_description options("Application Parameters",
                                lunchbox::term::getSize().first);
    addOption(options, ANIMATION_PARAM,
              "Enable animation mode (optional frame delta for animation "
              "speed, use --animation=-<int>  for reverse animation)",
              animation, 1);
    addOption(options, ANIMATION_FPS_PARAM,
              "Animation frames per second. By default (0), it will request a "
              "new frame as soon as the previous one is done",
              animationFPS);
    addOption(options, ANIMATION_FOLLOW_DATA_PARAM,
              "Enable animation and follow volume data stream (overrides "
              "--animation=value)",
              false);
    addOption(options, FRAMES_PARAM, "Frames to render [start end)", frames);
    addOption(options, NUMFRAMES_PARAM, "Maximum nuber of frames to render",
              maxFrames);
    addOption(options, CAMERAPOS_PARAM, "Camera position", cameraPosition);
    addOption(options, CAMERALOOKAT_PARAM, "Camera orientation", cameraLookAt);
    addOption(options, DATAFILE_PARAM, DataSource::getDescriptions(),
              std::string());
    addOption(options, TRANSFERFUNCTION_PARAM,
              ".1dt transfer function file (from ImageVis3D)",
              transferFunction);
    return options;
}

std::string ApplicationParameters::getHelp()
{
    std::stringstream os;
    os << ApplicationParameters()._getOptions();
    return os.str();
}
}
