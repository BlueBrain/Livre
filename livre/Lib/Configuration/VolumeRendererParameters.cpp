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

#include <livre/Lib/Configuration/VolumeRendererParameters.h>

namespace livre
{

const std::string SCREENSPACEERROR_PARAM = "sse";
const std::string SYNCHRONOUSMODE_PARAM = "synchronous";
const std::string GPUCACHEMEM_PARAM = "gpu-cache-mem";
const std::string CPUCACHEMEM_PARAM = "cpu-cache-mem";
const std::string MINLOD_PARAM = "min-lod";
const std::string MAXLOD_PARAM = "max-lod";
const std::string SAMPLESPERRAY_PARAM = "samples-per-ray";
const std::string TRANSFERFUNCTION_PARAM = "transfer-function";

VolumeRendererParameters::VolumeRendererParameters()
    : Parameters( "Volume Renderer Parameters" )
    , synchronousMode( false )
#ifdef __i386__
    , screenSpaceError( 8.0f )
    , maxGPUCacheMemoryMB( 512u )
    , maxCPUCacheMemoryMB( 1024u )
#else
    , screenSpaceError( 4.0f )
    , maxGPUCacheMemoryMB( 3072u )
    , maxCPUCacheMemoryMB( 8192u )
#endif
    , minLOD( 0 )
    , maxLOD( ( NODEID_LEVEL_BITS << 1 ) + 1 )
    , samplesPerRay( 512u )
    , transferFunction()
{
    configuration_.addDescription( configGroupName_, GPUCACHEMEM_PARAM,
                                   "Maximum GPU cache memory (MB) - "
                                   "caches the texture data in GPU memory",
                                   maxGPUCacheMemoryMB );
    configuration_.addDescription( configGroupName_, CPUCACHEMEM_PARAM,
                                   "Maximum CPU cache memory (MB) - "
                                   "caches the volume data in CPU memory",
                                   maxCPUCacheMemoryMB );
    configuration_.addDescription( configGroupName_, SCREENSPACEERROR_PARAM,
                                   "Screen space error", screenSpaceError );
    configuration_.addDescription( configGroupName_, SYNCHRONOUSMODE_PARAM,
                                   "Enable synchronous mode", synchronousMode );
    configuration_.addDescription( configGroupName_, MINLOD_PARAM,
                                   "Minimum level of detail", minLOD );
    configuration_.addDescription( configGroupName_, MAXLOD_PARAM,
                                   "Maximum level of detail", maxLOD );
    configuration_.addDescription( configGroupName_, SAMPLESPERRAY_PARAM,
                                   "Number of samples per ray", samplesPerRay );
    configuration_.addDescription( configGroupName_, TRANSFERFUNCTION_PARAM,
                                   "Path to transfer function file", transferFunction );
}

void VolumeRendererParameters::deserialize( co::DataIStream &is, const uint64_t )
{
    is >> screenSpaceError
       >> synchronousMode
       >> maxGPUCacheMemoryMB
       >> maxCPUCacheMemoryMB
       >> minLOD
       >> maxLOD
       >> samplesPerRay
       >> transferFunction;
}

void VolumeRendererParameters::serialize( co::DataOStream &os, const uint64_t )
{
    os << screenSpaceError
       << synchronousMode
       << maxGPUCacheMemoryMB
       << maxCPUCacheMemoryMB
       << minLOD
       << maxLOD
       << samplesPerRay
       << transferFunction;
}

VolumeRendererParameters &VolumeRendererParameters::operator=(
        const VolumeRendererParameters &rhs )
{
    if( this == &rhs )
        return *this;

    screenSpaceError = rhs.screenSpaceError;
    synchronousMode = rhs.synchronousMode;
    maxGPUCacheMemoryMB = rhs.maxGPUCacheMemoryMB;
    maxCPUCacheMemoryMB = rhs.maxCPUCacheMemoryMB;
    minLOD = rhs.minLOD;
    maxLOD = rhs.maxLOD;
    samplesPerRay = rhs.samplesPerRay;
    transferFunction = rhs.transferFunction;
    setDirty( DIRTY_ALL );

    return *this;
}

void VolumeRendererParameters::initialize_()
{
    configuration_.getValue( SYNCHRONOUSMODE_PARAM, synchronousMode );
    configuration_.getValue( SCREENSPACEERROR_PARAM, screenSpaceError );
    configuration_.getValue( GPUCACHEMEM_PARAM, maxGPUCacheMemoryMB );
    configuration_.getValue( CPUCACHEMEM_PARAM, maxCPUCacheMemoryMB);
    configuration_.getValue( MINLOD_PARAM, minLOD );
    configuration_.getValue( MAXLOD_PARAM, maxLOD );
    configuration_.getValue( SAMPLESPERRAY_PARAM, samplesPerRay );
    configuration_.getValue( TRANSFERFUNCTION_PARAM, transferFunction );
    setDirty( DIRTY_ALL );
}

} //Livre
