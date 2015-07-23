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

    Frustum( );

    /**
     * @param id The plane id.
     * @return The frustum plane information in world coordinates.
     */
    const Plane& getWPlane( const PlaneId id ) const;

    /**
     * @param id The plane id.
     * @return The frustum plane information in modelview coordinates.
     */
    const Plane& getMVPlane( const PlaneId id ) const;

    /**
     * @param worldBox AABB box.
     * @return True if box is intersecting or in the frustum.
     */
    bool boxInFrustum( const Boxf &worldBox ) const;

    /**
     * @return True if Frustum is initialized
     */
    bool isInitialized( ) const;

    /**
     * Initializes the Frustum parameters from the modelview and projection matrices.
     * @param modelViewMatrix 4x4 modelview matrix.
     * @param projectionMatrix 4x4 projection matrix,
     */
    void initialize(const Matrix4f& modelViewMatrix,
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
    void initialize( const Matrix4f& modelViewMatrix,
                     float l, float r,
                     float b, float t,
                     float n, float f );
    /**
     * Resets the Frustum.
     */
    void reset( );

    /**
     * @return The modelview matrix.
     */
    const Matrix4f& getModelViewMatrix( ) const;

    /**
     * @return The projection matrix.
     */
    const Matrix4f& getProjectionMatrix( ) const;

    /**
     * @return The inverse modelview matrix.
     */
    const Matrix4f& getInvModelViewMatrix( ) const;

    /**
     * @return The inverse projection matrix.
     */
    const Matrix4f& getInvProjectionMatrix( ) const;

    /**
     * @return The modelview projection matrix.
     */
    const Matrix4f& getModelViewProjectionMatrix( ) const;

    /**
     * @return The FOV in radians, only meaningful if frustum is symmetric.
     */
    float getFOV( ) const;

    /**
     * @param planeId The plane to get the distance of.
     * @return The plane distance to eye.
     */
    float getFrustumLimits( const PlaneId planeId ) const;

    /**
     * @return The eye coordinates in world space.
     */
    const Vector3f& getEyeCoords( ) const;

    /**
     * @return The viewing direction in world space.
     */
    const Vector3f& getViewDir( ) const;

    /**
     * @param planeId The plane id.
     * @return The given plane center in world space.
     */
    const Vector4f& getPlaneCenter( const PlaneId planeId ) const;

    /** @return True if the two frustums are the same. */
    bool operator == ( const Frustum& frustum ) const;

    /** @return True if the two frustums are not the same. */
    bool operator != ( const Frustum& frustum ) const
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
