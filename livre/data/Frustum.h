/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _Frustum_h_
#define _Frustum_h_

#include <livre/data/api.h>
#include <livre/data/types.h>
#include <vmmlib/frustum.hpp>       // base class
#include <vmmlib/frustumCuller.hpp> // member

#include <memory>

namespace livre
{
/**
 * The Frustum class holds the information about the viewing frustum in world,
 * eye and projection spaces.
 * All information is kept in OpenGL default coordinate systems.
 */
class Frustum : public Frustumf
{
public:
    /**
     * Initializes the Frustum parameters from the modelview and projection
     * matrices.
     * @param modelViewMatrix 4x4 modelview matrix.
     * @param projectionMatrix 4x4 projection matrix,
     */
    LIVREDATA_API Frustum(const Matrix4f& modelViewMatrix,
                          const Matrix4f& projectionMatrix);
    LIVREDATA_API ~Frustum();

    /** @return The frustum plane information in world coordinates. */
    LIVREDATA_API const Plane& getNearPlane() const;

    /**
     * @param worldBox AABB box.
     * @return True if box is intersecting or in the frustum.
     */
    LIVREDATA_API bool isInFrustum(const Boxf& worldBox) const;

    /**
     * @return The modelview matrix.
     */
    LIVREDATA_API const Matrix4f& getMVMatrix() const;

    /**
     * @return The projection matrix.
     */
    LIVREDATA_API const Matrix4f& getProjMatrix() const;

    /**
     * @return The inverse modelview matrix.
     */
    LIVREDATA_API const Matrix4f& getInvMVMatrix() const;

    /**
     * @return The inverse projection matrix.
     */
    LIVREDATA_API const Matrix4f& getInvProjMatrix() const;

    /**
     * @return The modelview projection matrix.
     */
    LIVREDATA_API Matrix4f getMVPMatrix() const;

    /**
     * @return The inverse transposed model-view matrix.
     */
    LIVREDATA_API Matrix4f getNormalMatrix() const;

    /**
     * @return The eye coordinates in world space.
     */
    LIVREDATA_API const Vector3f& getEyePos() const;

    /**
     * @return The eye direction in world space.
     */
    LIVREDATA_API const Vector3f& getViewDir() const;

    /** @return True if the two frustums are the same. */
    LIVREDATA_API bool operator==(const Frustum& rhs) const;

    /** @return True if the two frustums are not the same. */
    LIVREDATA_API bool operator!=(const Frustum& rhs) const
    {
        return !(*this == rhs);
    }

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};
}

#endif // _Frustum_h_
