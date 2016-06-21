/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Stefan.Eilemann@epfl.ch
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

#ifndef _FrameInfo_h_
#define _FrameInfo_h_

#include <livre/core/api.h>
#include <livre/core/cache/CacheObject.h> // member
#include <livre/core/data/NodeId.h> // member
#include <livre/core/render/Frustum.h> // member

namespace livre
{
/** Keeps the frame information. */
struct FrameInfo
{
    LIVRECORE_API FrameInfo( const Frustum& frustum,
                             const uint32_t frameId,
                             const uint32_t id );

    Frustum frustum; //!< The current frustum.
    uint32_t frameId ; //!< The current frame id for livre data sources.
    uint32_t id; //!< The unique id for each rendered frame

    size_t nAvailable; //!< Number of available nodes
    size_t nNotAvailable; //!< Number of not available nodes
};
}

#endif
