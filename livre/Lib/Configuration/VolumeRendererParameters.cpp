/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

const std::string DATACACHEMEM_PARAM = "data-cache-mem";
const std::string SCREENSPACEERROR_PARAM = "sse";
const std::string SYNCHRONOUSMODE_PARAM = "synchronous";
const std::string TEXTURECACHEMEM_PARAM = "texture-cache-mem";
const std::string TEXTUREDATACACHEMEM_PARAM = "texturedata-cache-mem";
const std::string MINLOD_PARAM = "min-lod";
const std::string MAXLOD_PARAM = "max-lod";

VolumeRendererParameters::VolumeRendererParameters()
    : Parameters( "Volume Renderer Parameters" )
    , renderStrategy( RS_ANY_FRAME )
    , synchronousModeEnabled( false )
    , screenSpaceError( 1.0f )
    , maxDataMemoryMB( 1024u )
    , maxTextureMemoryMB( 3072u )
    , maxTextureDataMemoryMB( 8192u )
    , minLOD( 0 )
    , maxLOD( ( NODEID_LEVEL_BITS << 1 ) + 1 )
{
    configuration_.addDescription( configGroupName_, DATACACHEMEM_PARAM,
                                   "Maximum data cache memory (MB) - "
                                   "caches the raw data read from I/O in system memory",
                                   maxDataMemoryMB );
    configuration_.addDescription( configGroupName_, TEXTURECACHEMEM_PARAM,
                                   "Maximum texture cache memory (MB) - "
                                   "caches the texture data on GPU memory",
                                   maxTextureMemoryMB );
    configuration_.addDescription( configGroupName_, TEXTUREDATACACHEMEM_PARAM,
                                   "Maximum texture data cache memory (MB) - "
                                   "caches the data that has been converted into internal texture "
                                   "format in system memory",
                                   maxTextureDataMemoryMB );
    configuration_.addDescription( configGroupName_, SCREENSPACEERROR_PARAM,
                                   "Screen space error", screenSpaceError );
    configuration_.addDescription( configGroupName_, SYNCHRONOUSMODE_PARAM,
                                   "Enable synchronous mode", synchronousModeEnabled );
    configuration_.addDescription( configGroupName_, MINLOD_PARAM,
                                   "Minimum level of detail", minLOD );
    configuration_.addDescription( configGroupName_, MAXLOD_PARAM,
                                   "Maximum level of detail", maxLOD );
}

void VolumeRendererParameters::deserialize( co::DataIStream &is, const uint64_t )
{
    is >> renderStrategy
       >> screenSpaceError
       >> maxDataMemoryMB
       >> maxTextureMemoryMB
       >> maxTextureDataMemoryMB
       >> minLOD
       >> maxLOD;
}

void VolumeRendererParameters::serialize( co::DataOStream &os, const uint64_t )
{
    os << renderStrategy
       << screenSpaceError
       << maxDataMemoryMB
       << maxTextureMemoryMB
       << maxTextureDataMemoryMB
       << minLOD
       << maxLOD;
}

VolumeRendererParameters &VolumeRendererParameters::operator=(
        const VolumeRendererParameters &volumeRendererParameters )
{
    renderStrategy = volumeRendererParameters.renderStrategy;
    screenSpaceError = volumeRendererParameters.screenSpaceError;
    maxDataMemoryMB = volumeRendererParameters.maxDataMemoryMB;
    maxTextureMemoryMB = volumeRendererParameters.maxTextureMemoryMB;
    maxTextureDataMemoryMB = volumeRendererParameters.maxTextureDataMemoryMB;
    minLOD = volumeRendererParameters.minLOD;
    maxLOD = volumeRendererParameters.maxLOD;
    return *this;
}

void VolumeRendererParameters::initialize_()
{
    configuration_.getValue( SYNCHRONOUSMODE_PARAM, synchronousModeEnabled );
    if( synchronousModeEnabled )
        renderStrategy = RS_FULL_FRAME;
    configuration_.getValue( SCREENSPACEERROR_PARAM, screenSpaceError );
    configuration_.getValue( DATACACHEMEM_PARAM, maxDataMemoryMB );
    configuration_.getValue( TEXTURECACHEMEM_PARAM, maxTextureMemoryMB );
    configuration_.getValue( TEXTUREDATACACHEMEM_PARAM, maxTextureDataMemoryMB );
    configuration_.getValue( MINLOD_PARAM, minLOD );
    configuration_.getValue( MAXLOD_PARAM, maxLOD );
}

} //Livre

namespace lunchbox
{
template<> inline void byteswap( livre::RenderStrategy& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }
}
