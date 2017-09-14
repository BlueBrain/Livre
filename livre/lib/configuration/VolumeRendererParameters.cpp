/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include "VolumeRendererParameters.h"

#include <lunchbox/term.h>

namespace livre
{
namespace
{
const char SCREENSPACEERROR_PARAM[] = "sse";
const char SYNCHRONOUSMODE_PARAM[] = "synchronous";
const char GPUCACHEMEM_PARAM[] = "gpu-cache-mem";
const char CPUCACHEMEM_PARAM[] = "cpu-cache-mem";
const char MINLOD_PARAM[] = "min-lod";
const char MAXLOD_PARAM[] = "max-lod";
const char SAMPLESPERRAY_PARAM[] = "samples-per-ray";
const char LINEARFILTERING_PARAM[] = "linear-filtering";
}

VolumeRendererParameters::VolumeRendererParameters()
{
#ifdef __i386__
    setSSE(8.0f);
    setMaxGPUCacheMemoryMB(384u);
    setMaxCPUCacheMemoryMB(768u);
#endif
}

VolumeRendererParameters::VolumeRendererParameters(const int32_t argc,
                                                   const char* const argv[])
{
#ifdef __i386__
    setSSE(8.0f);
    setMaxGPUCacheMemoryMB(384u);
    setMaxCPUCacheMemoryMB(768u);
#endif

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
            std::string("Error parsing renderer command line arguments") +
            exception.what()));
    }

    setSynchronousMode(vm[SYNCHRONOUSMODE_PARAM].as<bool>());
    setScreenSpaceError(vm[SCREENSPACEERROR_PARAM].as<float>());
    setMaxGpuCacheMemory(vm[GPUCACHEMEM_PARAM].as<uint64_t>());
    setMaxCpuCacheMemory(vm[CPUCACHEMEM_PARAM].as<uint64_t>());
    setMinLod(vm[MINLOD_PARAM].as<uint32_t>());
    setMaxLod(vm[MAXLOD_PARAM].as<uint32_t>());
    setSamplesPerRay(vm[SAMPLESPERRAY_PARAM].as<uint32_t>());
    setLinearFiltering(vm[LINEARFILTERING_PARAM].as<bool>());
}

options_description VolumeRendererParameters::_getOptions() const
{
    options_description options("Volume Renderer Parameters",
                                lunchbox::term::getSize().first);
    addOption(options, GPUCACHEMEM_PARAM,
              "Maximum GPU cache memory (MB) for textures",
              getMaxGpuCacheMemory());
    addOption(options, CPUCACHEMEM_PARAM,
              "Maximum CPU cache memory (MB) for textures",
              getMaxCpuCacheMemory());
    addOption(options, SCREENSPACEERROR_PARAM, "Screen space error",
              getScreenSpaceError());
    addOption(options, SYNCHRONOUSMODE_PARAM, "Enable synchronous mode",
              getSynchronousMode());
    addOption(options, MINLOD_PARAM, "Minimum level of detail", getMinLod());
    addOption(options, MAXLOD_PARAM, "Maximum level of detail", getMaxLod());
    addOption(options, SAMPLESPERRAY_PARAM,
              "Number of samples per ray. The value of 0 (default) sets the "
              "number of samples automatically, based on the volume size",
              getSamplesPerRay());
    addOption(options, LINEARFILTERING_PARAM,
              "Use linear texture filtering instead of nearest", false);
    return options;
}

std::string VolumeRendererParameters::getHelp()
{
    std::stringstream os;
    os << VolumeRendererParameters()._getOptions();
    return os.str();
}
} // Livre
