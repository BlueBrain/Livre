/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#include <livre/core/configuration/Parameters.h>
#include <livre/lib/api.h>
#include <livre/core/mathTypes.h>

namespace livre
{

/**
 * The ApplicationParameters struct keeps the parameters for main application.
 */
struct ApplicationParameters : public Parameters
{
    LIVRE_API ApplicationParameters();

    Vector3f cameraPosition; //!< Camera position in world space.
    Vector3f cameraLookAt; //!< Camera orientation (lookAt vector) in world space.
    Vector2ui frames; //!< Range of frames to render: [start end).
    uint32_t maxFrames; //!< Max number of frames to render.
    int32_t animation; //!< animation forward/backward speed
    bool isResident; //!< Is the main app resident.
    std::string dataFileName; //!< Data file name.
    std::string transferFunction; //!< Path to transfer function file

    /** @param parameters The source parameters. */
    LIVRE_API ApplicationParameters& operator=( const ApplicationParameters& parameters );

private:
    void initialize_() final;
};

}

#endif // _ApplicationParameters_h_
