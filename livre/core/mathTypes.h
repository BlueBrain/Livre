/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <livre/core/types.h>
#include <eq/fabric/vmmlib.h>
#include <lunchbox/lunchbox.h>

namespace livre
{

/**
 * AABB definitions
 */
typedef vmml::AABB< float > Boxf; //!< float AABB box.
typedef vmml::AABB< int32_t > Boxi; //!< integer AABB box.
typedef vmml::AABB< uint32_t > Boxui; //!< integer AABB box.

/**
 * Matrix definitions
 */
using eq::fabric::Matrix3f; //!< A 3x3 float matrix.
using eq::fabric::Matrix4f; //!< A 4x4 float matrix.

using eq::fabric::Matrix3d; //!< A 3x3 double matrix.
using eq::fabric::Matrix4d; //!< A 4x4 double matrix.

/**
 * Vector definitions
 */
using eq::fabric::Vector2i; //!< A two-component integer vector.
using eq::fabric::Vector3i; //!< A three-component integer vector.
using eq::fabric::Vector4i; //!< A four-component integer vector.

typedef vmml::vector< 2, uint32_t > Vector2ui; //!< A two-component integer vector.
typedef vmml::vector< 3, uint32_t > Vector3ui; //!< A three-component integer vector.
typedef vmml::vector< 4, uint32_t > Vector4ui; //!< A four-component integer vector.

using eq::fabric::Vector2f; //!< A two-component float vector.
using eq::fabric::Vector3f; //!< A three-component float vector.
using eq::fabric::Vector3d; //!< A three-component double vector.
using eq::fabric::Vector4d; //!< A four-component double vector.
using eq::fabric::Vector4f; //!< A four-component float vector.

/**
 * Frustum definitions
 */
typedef vmml::frustum< float > Frustumf; //!< Float frustum.

typedef vmml::frustum_culler< float > FrustumCullerf; //!< Float frustum culler.

/**
 * Quaternion definitions
 */
typedef vmml::quaternion< float > Quaternionf; //!< Float quaternion.

/**
 * Viewport definitions
 */
typedef vmml::Vector4i PixelViewport;
typedef vmml::Vector4f Viewport;

/**
 * Other definitions
 */
typedef std::pair< Vector3f, Vector3f > MaxMinPair;

}

#endif // _mathTypes_h_
