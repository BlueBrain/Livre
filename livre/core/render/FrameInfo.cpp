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

#include <livre/core/render/FrameInfo.h>

namespace livre
{

FrameInfo::FrameInfo( const Frustum& frustum_,
                      const uint32_t timeStep_,
                      const uint32_t frameId_ )
    : frustum( frustum_ )
    , timeStep( timeStep_ )
    , frameId( frameId_ )
{}

FrameInfo::FrameInfo()
    : frustum( Matrix4f(), Matrix4f( ))
    , timeStep( INVALID_TIMESTEP )
    , frameId( INVALID_FRAMEID )
{}

}
