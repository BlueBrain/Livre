/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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
#include <livre/core/Maths/Plane.h>

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
    LIVRECORE_API const Plane& getWPlane( const PlaneId id ) const;

    /**
     * @param id The plane id.
     * @return The frustum plane information in modelview coordinates.
     */
    LIVRECORE_API const Plane& getMVPlane( const PlaneId id ) const;

    /**
     * @param worldBox AABB box.
     * @return True if box is intersecting or in the frustum.
     */
    LIVRECORE_API bool boxInFrustum( const Boxf &worldBox ) const;

    /**
     * @return True if Frustum is initialized
     */
    LIVRECORE_API bool isInitialized( ) const;

    /**
     * Initializes the Frustum parameters from the modelview and projection matrices.
     * @param modelViewMatrix 4x4 modelview matrix.
     * @param projectionMatrix 4x4 projection matrix,
     */
    LIVRECORE_API void initialize(const Matrix4f& modelViewMatrix,
                              const Matrix4f& projectionMatrix );

    /**
     * Initializes the Frustum parameters from the modelview matrix and plane distances to eye.
     * @param modelViewMatrix 4x4 modelview matrix.
     * @param l left plane distance.
     * @param r right plane distance.
     * @param b bottom plane distance.
     * @param t top plane distance.
     * @param n near plane distance.
     * @param f far plane distance.
     */
    LIVRECORE_API void initialize( const Matrix4f& modelViewMatrix,
                               float l, float r,
                               float b, float t,
                               float n, float f );
    /**
     * Resets the Frustum.
     */
    LIVRECORE_API void reset( );

    /**
     * @return The modelview matrix.
     */
    LIVRECORE_API const Matrix4f& getModelViewMatrix( ) const;

    /**
     * @return The projection matrix.
     */
    LIVRECORE_API const Matrix4f& getProjectionMatrix( ) const;

    /**
     * @return The inverse modelview matrix.
     */
    LIVRECORE_API const Matrix4f& getInvModelViewMatrix( ) const;

    /**
     * @return The inverse projection matrix.
     */
    LIVRECORE_API const Matrix4f& getInvProjectionMatrix( ) const;

    /**
     * @return The modelview projection matrix.
     */
    LIVRECORE_API const Matrix4f& getModelViewProjectionMatrix( ) const;

    /**
     * @return The FOV in radians, only meaningful if frustum is symmetric.
     */
    LIVRECORE_API float getFOV( ) const;

    /**
     * @param planeId The plane to get the distance of.
     * @return The plane distance to eye.
     */
    LIVRECORE_API float getFrustumLimits( const PlaneId planeId ) const;

    /**
     * @return The eye coordinates in world space.
     */
    LIVRECORE_API const Vector3f& getEyeCoords( ) const;

    /**
     * @return The viewing direction in world space.
     */
    LIVRECORE_API const Vector3f& getViewDir( ) const;

    /**
     * @param planeId The plane id.
     * @return The given plane center in world space.
     */
    LIVRECORE_API const Vector4f& getPlaneCenter( const PlaneId planeId ) const;

    /** @return True if the two frustums are the same. */
    LIVRECORE_API bool operator == ( const Frustum& frustum ) const;

    /** @return True if the two frustums are not the same. */
    LIVRECORE_API bool operator != ( const Frustum& frustum ) const
        { return !(*this == frustum); }

private:

    void computeFrustumVertices_( Vector3f frustumVertices[],
                                  Vector3f frustumNormals[] ) const;
    void computeLimitsFromProjectionMatrix_( );
    void initializePlanes_( const Matrix4f &matrix, Plane *planes );
    void initialize_( );
    void initializePlaneCenters_( );

    Plane wPlanes_[ 6 ];
    Plane mvPlanes_[ 6 ];
    Matrix4f mvpMatrix_;
    Matrix4f modelViewMatrix_;
    Matrix4f invModelViewMatrix_;
    Matrix4f projectionMatrix_;
    Matrix4f invProjectionMatrix_;
    bool isInitialized_;
    float fovy_;
    float fovx_;
    Vector3f eye_;
    Vector3f viewDir_;
    Vector4f planeCenters_[ 6 ];
    Frustumf vmmlFrustum_;
    FrustumCullerf vmmlFrustumCuller_;
};

}

#endif // _Frustum_h_
