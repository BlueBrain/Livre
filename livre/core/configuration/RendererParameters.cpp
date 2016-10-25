/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include "RendererParameters.h"

namespace livre
{

const std::string SCREENSPACEERROR_PARAM = "sse";
const std::string SYNCHRONOUSMODE_PARAM = "synchronous";
const std::string GPUCACHEMEM_PARAM = "gpu-cache-mem";
const std::string CPUCACHEMEM_PARAM = "cpu-cache-mem";
const std::string MINLOD_PARAM = "min-lod";
const std::string MAXLOD_PARAM = "max-lod";
const std::string SAMPLESPERRAY_PARAM = "samples-per-ray";
const std::string SAMPLESPERPIXEL_PARAM = "samples-per-pixel";

RendererParameters::RendererParameters()
    : Parameters( "Volume Renderer Parameters" )
{
    _configuration.addDescription( configGroupName_, GPUCACHEMEM_PARAM,
                                   "Maximum GPU cache memory (MB) - "
                                   "caches the texture data in GPU memory",
                                   getMaxGPUCacheMemoryMB( ));
    _configuration.addDescription( configGroupName_, CPUCACHEMEM_PARAM,
                                   "Maximum CPU cache memory (MB) - "
                                   "caches the volume data in CPU memory",
                                   getMaxCPUCacheMemoryMB( ));
    _configuration.addDescription( configGroupName_, SCREENSPACEERROR_PARAM,
                                   "Screen space error", getSSE( ));
    _configuration.addDescription( configGroupName_, SYNCHRONOUSMODE_PARAM,
                                   "Enable synchronous mode", getSynchronousMode( ));
    _configuration.addDescription( configGroupName_, MINLOD_PARAM,
                                   "Minimum level of detail", getMinLOD( ));
    _configuration.addDescription( configGroupName_, MAXLOD_PARAM,
                                   "Maximum level of detail", getMaxLOD( ));
    _configuration.addDescription( configGroupName_, SAMPLESPERRAY_PARAM,
                                   "Number of samples per ray. The value of 0 (default) sets"
                                   " the number of samples automatically, based on the volume size",
                                   getSamplesPerRay( ));
    _configuration.addDescription( configGroupName_, SAMPLESPERPIXEL_PARAM,
                                   "Number of samples per pixel", getSamplesPerPixel( ));
}

void RendererParameters::_initialize()
{
    setSynchronousMode( _configuration.getValue( SYNCHRONOUSMODE_PARAM,
                                                 getSynchronousMode( )));
    setSSE( _configuration.getValue( SCREENSPACEERROR_PARAM, getSSE( )));
    setMaxGPUCacheMemoryMB( _configuration.getValue( GPUCACHEMEM_PARAM,
                                                     getMaxGPUCacheMemoryMB( )));
    setMaxCPUCacheMemoryMB( _configuration.getValue( CPUCACHEMEM_PARAM,
                                                     getMaxCPUCacheMemoryMB( )));
    setMinLOD( _configuration.getValue( MINLOD_PARAM, getMinLOD( )));
    setMaxLOD( _configuration.getValue( MAXLOD_PARAM, getMaxLOD( )));
    setSamplesPerRay( _configuration.getValue( SAMPLESPERRAY_PARAM,
                                               getSamplesPerRay( )));
    setSamplesPerPixel( _configuration.getValue( SAMPLESPERPIXEL_PARAM,
                                                 getSamplesPerPixel( )));
}

} //Livre
