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

#ifndef _EFPrefetchAlgorithmParameters_h_
#define _EFPrefetchAlgorithmParameters_h_

#include <livre/core/Configuration/Parameters.h>

namespace livre
{

/**
 * The EFPrefetchAlgorithmParameters struct is Expanded Frustum prefetch algorithm specific parameters.
 */
struct EFPrefetchAlgorithmParameters : public Parameters
{
    EFPrefetchAlgorithmParameters( );

    bool dynamicFrustumPlanesEnabled; //<! The frustum planes modifocation.
    float frustumExpansionSpeed; //<! Sets the expansion speed of frustum planes by translation.
    float rollExpansionSpeed; //<! Sets the speed of expansion caused by roll.
    float minPrefetchDistanceRatio; //<! Prefetch distance modification.
    float maxPrefetchDistanceRatio; //<! Prefetch distance modification.
    float optimizerPrecision; //<! Optimizer precision.
    bool waitUntilAllPrefetchBuffersAreFull; //<! Do not start rendering until all buffers are full or all data loaded.


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
     * @param parameters The source parameters.
     */
    EFPrefetchAlgorithmParameters& operator=( const EFPrefetchAlgorithmParameters& parameters );

protected:

    virtual void initialize_( );
};

}

#endif // _EFPrefetchAlgorithmParameters_h_
