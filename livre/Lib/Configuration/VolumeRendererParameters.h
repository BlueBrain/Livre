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

#ifndef _VolumeRendererParameters_h_
#define _VolumeRendererParameters_h_

#include <livre/core/Configuration/Parameters.h>

namespace livre
{

enum RenderStrategy
{
    RS_ANY_FRAME, //!< Render with data available
    RS_ITERATED_FULL_FRAME, //!< Render in between frames between two camera positions for all the needed data
    RS_FULL_FRAME //!< Render for all data at the current camera position, do not skip until it is satisfied
};

struct VolumeRendererParameters : public Parameters
{
    VolumeRendererParameters( );

    RenderStrategy renderStrategy; //!< Render strategy
    float screenSpaceError;  //!< Screen space error
    uint32_t maxDataMemoryMB;  //!< Settings for data cache
    uint32_t maxTextureMemoryMB; //!< Settings for texture cache
    uint32_t maxTextureDataMemoryMB; //!< Settings for texture cache
    uint32_t maxRenderMemoryMB; //!< Maximum render frustum
    bool skipLoadingEnabled; //!< Skip loading the data when a new frame data request arrives
    bool disableNormalization; //!< Normalize data
    bool dropAnimationFrames; //!< Drop animation frames

    /**
     * De-serializes the object from input stream.
     * @param dirtyBits The bits for elements can be marked
     * @param is Input stream.
     */
    virtual void deserialize( co::DataIStream& is, const uint64_t dirtyBits );

    /**
     * Serializes the object to output stream.
     * @param dirtyBits The bits for elements can be marked
     * @param os Output stream.
     */
    virtual void serialize( co::DataOStream& os, const uint64_t dirtyBits );

    /**
     * @param volumeRendererParameters The source parameters.
     */
    VolumeRendererParameters& operator=( const VolumeRendererParameters& volumeRendererParameters );

protected:

    virtual void initializeParameters_( );
};

}

#endif // _VolumeRendererParameters_h_
