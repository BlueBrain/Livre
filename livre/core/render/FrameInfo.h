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
                             const uint32_t frameId );
    /**
      * Clears the node information
      */
    LIVRECORE_API void clear();

    /**
      * Merges the node information from the other frame info
      * @param frameInfo source frame info
      * @return returns the current frame info
      */
    LIVRECORE_API FrameInfo& merge( const FrameInfo& frameInfo );

    NodeIds notAvailableRenderNodes; //!< The unavailable nodes for rendering.
    ConstCacheObjects renderNodes; //!< The list of nodes to be rendered.
    const Frustum& currentFrustum; //!< The current frustum.
    const uint32_t frameId ; //!< The current frustum.

};
}

#endif
