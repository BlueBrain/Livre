/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 *
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

#ifndef _mathTypes_h_
#define _mathTypes_h_

#include <vmmlib/types.hpp>
#include <vmmlib/matrix.hpp>
#include <vmmlib/vector.hpp> // inline constants

namespace livre
{

/**
 * AABB definitions
 */
typedef vmml::AABBf Boxf; //!< float AABB box.
typedef vmml::AABB< int32_t > Boxi; //!< integer AABB box.
typedef vmml::AABB< uint32_t > Boxui; //!< integer AABB box.

/**
 * Matrix definitions
 */
using vmml::Matrix3f; //!< A 3x3 float matrix.
using vmml::Matrix4f; //!< A 4x4 float matrix.

using vmml::Matrix3d; //!< A 3x3 double matrix.
using vmml::Matrix4d; //!< A 4x4 double matrix.

/**
 * Vector definitions
 */
using vmml::Vector2i; //!< A two-component integer vector.
using vmml::Vector3i; //!< A three-component integer vector.
using vmml::Vector4i; //!< A four-component integer vector.

using vmml::Vector2ui; //!< A two-component integer vector.
using vmml::Vector3ui; //!< A three-component integer vector.
using vmml::Vector4ui; //!< A four-component integer vector.

using vmml::Vector2f; //!< A two-component float vector.
using vmml::Vector3f; //!< A three-component float vector.
using vmml::Vector3d; //!< A three-component double vector.
using vmml::Vector4d; //!< A four-component double vector.
using vmml::Vector4f; //!< A four-component float vector.
using vmml::Frustumf; //!< Float frustum.
using vmml::FrustumCullerf; //!< Float frustum culler.
typedef Vector4f Plane;

/**
 * Quaternion definitions
 */
using vmml::Quaternionf; //!< Float quaternion.

/**
 * Viewport definitions
 */
typedef vmml::Vector4i PixelViewport;
typedef vmml::Vector4f Viewport;

/**
  * Definitions
  */
const uint32_t NODEID_TIMESTEP_BITS = 18; //>! see NodeId
const uint32_t INVALID_TIMESTEP = ( 1u << NODEID_TIMESTEP_BITS ) - 1u; //!< Invalid time step. 18 bits is on
const Vector2ui INVALID_FRAME_RANGE( INVALID_TIMESTEP );
const Vector2ui FULL_FRAME_RANGE( 0, INVALID_TIMESTEP );

/**
 * Vector definitions
 */
typedef std::vector< Vector3f > Vector3fs;

}

#endif // _mathTypes_h_
