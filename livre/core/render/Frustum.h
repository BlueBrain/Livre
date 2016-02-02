/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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


#include <livre/core/api.h>
#include <livre/core/mathTypes.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * The PlaneId enum is used to identify the planes of the Frustum. OpenGL function glFrustum() has the same
 * order.
 */
enum PlaneId
{
    PL_LEFT,
    PL_RIGHT,
    PL_BOTTOM,
    PL_TOP,
    PL_NEAR,
    PL_FAR
};

/**
 * The Frustum class holds the information about the viewing frustum in world, eye and projection spaces.
 * All information is kept in OpenGL default coordinate systems.
 */
class Frustum
{
public:
    LIVRECORE_API Frustum();

    /**
     * @param id The plane id.
     * @return The frustum plane information in world coordinates.
     */
    LIVRECORE_API const Plane& getNearPlane() const;

    /**
     * @param worldBox AABB box.
     * @return True if box is intersecting or in the frustum.
     */
    LIVRECORE_API bool boxInFrustum( const Boxf &worldBox ) const;

    /**
     * Initializes the Frustum parameters from the modelview and projection matrices.
     * @param modelViewMatrix 4x4 modelview matrix.
     * @param projectionMatrix 4x4 projection matrix,
     */
    LIVRECORE_API void setup( const Matrix4f& modelViewMatrix,
                              const Matrix4f& projectionMatrix );

    /**
     * @return The modelview matrix.
     */
    LIVRECORE_API const Matrix4f& getModelViewMatrix() const;

    /**
     * @return The projection matrix.
     */
    LIVRECORE_API const Matrix4f& getProjectionMatrix() const;

    /**
     * @return The inverse modelview matrix.
     */
    LIVRECORE_API const Matrix4f& getInvModelViewMatrix() const;

    /**
     * @return The inverse projection matrix.
     */
    LIVRECORE_API const Matrix4f& getInvProjectionMatrix() const;

    /**
     * @return The modelview projection matrix.
     */
    LIVRECORE_API Matrix4f getModelViewProjectionMatrix() const;

    /**
     * @param planeId The plane to get the distance of.
     * @return The plane distance to eye.
     */
    LIVRECORE_API float getFrustumLimits( const PlaneId planeId ) const;

    /**
     * @return The eye coordinates in world space.
     */
    LIVRECORE_API const Vector3f& getEyeCoords() const;

    /** @return True if the two frustums are the same. */
    LIVRECORE_API bool operator == ( const Frustum& frustum ) const;

    /** @return True if the two frustums are not the same. */
    LIVRECORE_API bool operator != ( const Frustum& frustum ) const
        { return !(*this == frustum); }

private:
    Matrix4f modelViewMatrix_;
    Matrix4f invModelViewMatrix_;
    Matrix4f projectionMatrix_;
    Matrix4f invProjectionMatrix_;
    Vector3f eye_;
    Frustumf vmmlFrustum_;
    FrustumCullerf _culler;

    void computeLimitsFromProjectionMatrix_();
};

}

#endif // _Frustum_h_
