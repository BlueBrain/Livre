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

#ifndef _ApplicationParameters_h_
#define _ApplicationParameters_h_

#include <livre/core/mathTypes.h>
#include <livre/core/Configuration/Parameters.h>

namespace livre
{

/**
 * The ApplicationParameters struct keeps the parameters for main application.
 */
struct ApplicationParameters : public Parameters
{
    ApplicationParameters();

    std::string dataFileName; //!< Data file name.
    bool animationEnabled; //!< Enable the animation mode.
    Vector2i frames; //!< Range of frames to render: 'start end'.
    bool isResident; //!< Is the main app resident.
    Vector3f cameraPosition; //!< Camera position in world space.
    bool syncCamera; //!< Synchronize camera with other applications using ZEQ.

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
     * @param parameters The source parameters.
     */
    ApplicationParameters& operator=( const ApplicationParameters& parameters );

protected:
    void initialize_() final;
};

}

#endif // _ApplicationParameters_h_
