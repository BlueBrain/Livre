/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

struct VolumeRendererParameters : public Parameters
{
    VolumeRendererParameters();

    bool synchronousMode; //!< Synchronous mode
    float screenSpaceError;  //!< Screen space error
    uint32_t maxGPUCacheMemoryMB; //!< Max memory for texture cache
    uint32_t maxCPUCacheMemoryMB; //!< Max memory for data cache
    uint32_t minLOD; //!< Minimum level of detail
    uint32_t maxLOD; //!< Maximum level of detail
    uint32_t samplesPerRay; //!< Number of samples per ray
    std::string transferFunction; //!< Path to transfer function file

    /**
     * De-serializes the object from input stream.
     * @param dirtyBits The bits for elements can be marked
     * @param is Input stream.
     */
    void deserialize( co::DataIStream& is, const uint64_t dirtyBits ) final;

    /**
     * Serializes the object to output stream.
     * @param dirtyBits The bits for elements can be marked
     * @param os Output stream.
     */
    void serialize( co::DataOStream& os, const uint64_t dirtyBits ) final;

    /**
     * @param volumeRendererParameters The source parameters.
     */
    VolumeRendererParameters& operator=( const VolumeRendererParameters& params );

protected:
    void initialize_() final;
};

}

#endif // _VolumeRendererParameters_h_
