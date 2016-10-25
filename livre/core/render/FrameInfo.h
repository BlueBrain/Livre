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
                             const uint32_t timeStep,
                             const uint32_t frameId );
    LIVRECORE_API FrameInfo();

    Frustum frustum; //!< The current frustum.
    uint32_t timeStep ; //!< The current time step for livre data sources.
    uint32_t frameId; //!< The unique id for each rendered frame
};


/** Keeps the number information of avalibility of the nodes in cache */
struct RenderStatistics
{
    LIVRECORE_API RenderStatistics()
     : nAvailable( 0 )
     , nNotAvailable( 0 )
     , nRenderAvailable( 0 )
    {}

    LIVRECORE_API RenderStatistics& operator+=( const RenderStatistics& na )
    {
        nAvailable += na.nAvailable;
        nNotAvailable += na.nNotAvailable;
        nRenderAvailable += na.nRenderAvailable;
        return *this;
    }

    size_t nAvailable; //!< Number of available nodes
    size_t nNotAvailable; //!< Number of not available nodes
    size_t nRenderAvailable; //!< Number of render nodes
};

}

#endif
