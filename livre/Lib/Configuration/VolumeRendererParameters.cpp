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

const char *SCREENSPACEERROR_PARAM = "sse";
const char *DATACACHEMEM_PARAM = "datacachemem";
const char *TEXTUREDATACACHEMEM_PARAM = "texturedatacachemem";
const char *TEXTURECACHEMEM_PARAM = "texturecachemem";
const char *MAXRENDERMEM_PARAM = "maxrendermem";

VolumeRendererParameters::VolumeRendererParameters()
    : Parameters( "Volume Renderer Parameters" ),
      renderStrategy( RS_ANY_FRAME ),
      screenSpaceError( 1.0f ),
      maxDataMemoryMB( 1024u ),
      maxTextureMemoryMB( 3072u ),
      maxTextureDataMemoryMB( 8192u ),
      maxRenderMemoryMB( 1536u ),
      skipLoadingEnabled( false ),
      disableNormalization( false ),
      dropAnimationFrames( false )
{
    configuration_.addDescription( configGroupName_, SCREENSPACEERROR_PARAM, "Screen space error", screenSpaceError );
    configuration_.addDescription( configGroupName_, DATACACHEMEM_PARAM,
                                  "Maximum data cache memory (MB)", maxDataMemoryMB );
    configuration_.addDescription( configGroupName_, "texturecachemem",
                                  "Maximum texture cache memory (MB)", maxTextureMemoryMB );
    configuration_.addDescription( configGroupName_, TEXTUREDATACACHEMEM_PARAM,
                                  "Maximum texture data cache memory (MB)", maxTextureDataMemoryMB );
    configuration_.addDescription( configGroupName_, MAXRENDERMEM_PARAM, "Maximum render memory",
                                   maxRenderMemoryMB );
}

void VolumeRendererParameters::deserialize( co::DataIStream &is, const uint64_t dirtyBits LB_UNUSED )
{
    is >> renderStrategy
       >> screenSpaceError
       >> maxDataMemoryMB
       >> maxTextureMemoryMB
       >> maxTextureDataMemoryMB
       >> skipLoadingEnabled
       >> disableNormalization
       >> dropAnimationFrames
       >> maxRenderMemoryMB;
}

void VolumeRendererParameters::serialize( co::DataOStream &os, const uint64_t dirtyBits LB_UNUSED )
{
    os << renderStrategy
       << screenSpaceError
       << maxDataMemoryMB
       << maxTextureMemoryMB
       << maxTextureDataMemoryMB
       << skipLoadingEnabled
       << disableNormalization
       << dropAnimationFrames
       << maxRenderMemoryMB;
}

VolumeRendererParameters &VolumeRendererParameters::operator=(
        const VolumeRendererParameters &volumeRendererParameters )
{
    renderStrategy = volumeRendererParameters.renderStrategy;
    screenSpaceError = volumeRendererParameters.screenSpaceError;
    maxDataMemoryMB = volumeRendererParameters.maxDataMemoryMB;
    maxTextureMemoryMB = volumeRendererParameters.maxTextureMemoryMB;
    maxTextureDataMemoryMB = volumeRendererParameters.maxTextureDataMemoryMB;
    skipLoadingEnabled = volumeRendererParameters.skipLoadingEnabled;
    disableNormalization = volumeRendererParameters.disableNormalization;
    dropAnimationFrames = volumeRendererParameters.dropAnimationFrames;
    maxRenderMemoryMB = volumeRendererParameters.maxRenderMemoryMB;

    return *this;
}

void VolumeRendererParameters::initializeParameters_()
{
    renderStrategy = RS_ANY_FRAME;
    configuration_.getValue( SCREENSPACEERROR_PARAM, screenSpaceError );
    configuration_.getValue( DATACACHEMEM_PARAM, maxDataMemoryMB );
    configuration_.getValue( TEXTURECACHEMEM_PARAM, maxTextureMemoryMB );
    configuration_.getValue( TEXTUREDATACACHEMEM_PARAM, maxTextureDataMemoryMB );
    configuration_.getValue( MAXRENDERMEM_PARAM, maxRenderMemoryMB );
}

} //Livre

namespace lunchbox
{
template<> inline void byteswap( livre::RenderStrategy& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }
}
