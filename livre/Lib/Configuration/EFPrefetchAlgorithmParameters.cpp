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

#include <livre/Lib/Configuration/EFPrefetchAlgorithmParameters.h>

namespace livre
{

EFPrefetchAlgorithmParameters::EFPrefetchAlgorithmParameters()
    : Parameters( "Expanded Frustum Algorithm Parameters" )
{

    dynamicFrustumPlanesEnabled = true;
    frustumExpansionSpeed = 1.0f / 100.0f;
    rollExpansionSpeed = 1.0f / 100.0f;
    minPrefetchDistanceRatio = 0.5f;
    maxPrefetchDistanceRatio = 1.5f;
    optimizerPrecision = 0.001f;
    waitUntilAllPrefetchBuffersAreFull = false;
}

void EFPrefetchAlgorithmParameters::serialize( co::DataOStream &os, const uint64_t dirtyBits LB_UNUSED )
{
    os << dynamicFrustumPlanesEnabled
       << frustumExpansionSpeed
       << rollExpansionSpeed
       << minPrefetchDistanceRatio
       << maxPrefetchDistanceRatio
       << optimizerPrecision
       << waitUntilAllPrefetchBuffersAreFull;
}

void EFPrefetchAlgorithmParameters::deserialize( co::DataIStream &is, const uint64_t dirtyBits LB_UNUSED )
{
    is >> dynamicFrustumPlanesEnabled
       >> frustumExpansionSpeed
       >> rollExpansionSpeed
       >> minPrefetchDistanceRatio
       >> maxPrefetchDistanceRatio
       >> optimizerPrecision
       >> waitUntilAllPrefetchBuffersAreFull;
}

EFPrefetchAlgorithmParameters &EFPrefetchAlgorithmParameters::operator=(
        const EFPrefetchAlgorithmParameters &parameters )
{
    dynamicFrustumPlanesEnabled = parameters.dynamicFrustumPlanesEnabled;
    frustumExpansionSpeed = parameters.frustumExpansionSpeed;
    rollExpansionSpeed = parameters.rollExpansionSpeed;
    minPrefetchDistanceRatio = parameters.minPrefetchDistanceRatio;
    maxPrefetchDistanceRatio = parameters.maxPrefetchDistanceRatio;
    optimizerPrecision = parameters.optimizerPrecision;
    waitUntilAllPrefetchBuffersAreFull = parameters.waitUntilAllPrefetchBuffersAreFull;

    return *this;
}

void EFPrefetchAlgorithmParameters::initialize_()
{
}

}
