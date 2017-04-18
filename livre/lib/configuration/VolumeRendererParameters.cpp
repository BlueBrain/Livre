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

namespace livre
{
const std::string SCREENSPACEERROR_PARAM = "sse";
const std::string SYNCHRONOUSMODE_PARAM = "synchronous";
const std::string GPUCACHEMEM_PARAM = "gpu-cache-mem";
const std::string CPUCACHEMEM_PARAM = "cpu-cache-mem";
const std::string MINLOD_PARAM = "min-lod";
const std::string MAXLOD_PARAM = "max-lod";
const std::string SAMPLESPERRAY_PARAM = "samples-per-ray";
const std::string LINEARFILTERING_PARAM = "linear-filtering";

VolumeRendererParameters::VolumeRendererParameters()
    : Parameters("Volume Renderer Parameters")
{
#ifdef __i386__
    setSSE(8.0f);
    setMaxGPUCacheMemoryMB(384u);
    setMaxCPUCacheMemoryMB(768u);
#endif

    configuration_.addDescription(configGroupName_, GPUCACHEMEM_PARAM,
                                  "Maximum GPU cache memory (MB) - "
                                  "caches the texture data in GPU memory",
                                  getMaxGpuCacheMemory());
    configuration_.addDescription(configGroupName_, CPUCACHEMEM_PARAM,
                                  "Maximum CPU cache memory (MB) - "
                                  "caches the volume data in CPU memory",
                                  getMaxCpuCacheMemory());
    configuration_.addDescription(configGroupName_, SCREENSPACEERROR_PARAM,
                                  "Screen space error", getScreenSpaceError());
    configuration_.addDescription(configGroupName_, SYNCHRONOUSMODE_PARAM,
                                  "Enable synchronous mode",
                                  getSynchronousMode());
    configuration_.addDescription(configGroupName_, MINLOD_PARAM,
                                  "Minimum level of detail", getMinLod());
    configuration_.addDescription(configGroupName_, MAXLOD_PARAM,
                                  "Maximum level of detail", getMaxLod());
    configuration_.addDescription(
        configGroupName_, SAMPLESPERRAY_PARAM,
        "Number of samples per ray. The value of 0 (default) sets"
        " the number of samples automatically, based on the volume size",
        getSamplesPerRay());
    configuration_.addDescription(
        configGroupName_, LINEARFILTERING_PARAM,
        "Use linear texture filtering instead of nearest", false);
}

void VolumeRendererParameters::initialize_()
{
    setSynchronousMode(
        configuration_.getValue(SYNCHRONOUSMODE_PARAM, getSynchronousMode()));
    setScreenSpaceError(
        configuration_.getValue(SCREENSPACEERROR_PARAM, getScreenSpaceError()));
    setMaxGpuCacheMemory(
        configuration_.getValue(GPUCACHEMEM_PARAM, getMaxGpuCacheMemory()));
    setMaxCpuCacheMemory(
        configuration_.getValue(CPUCACHEMEM_PARAM, getMaxCpuCacheMemory()));
    setMinLod(configuration_.getValue(MINLOD_PARAM, getMinLod()));
    setMaxLod(configuration_.getValue(MAXLOD_PARAM, getMaxLod()));
    setSamplesPerRay(
        configuration_.getValue(SAMPLESPERRAY_PARAM, getSamplesPerRay()));
    setLinearFiltering(
        configuration_.getValue(LINEARFILTERING_PARAM, getLinearFiltering()));
}

} // Livre
