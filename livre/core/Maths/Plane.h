/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _Plane_h_
#define _Plane_h_

#include <livre/core/api.h>
#include <livre/core/mathTypes.h>

namespace livre
{

/**
 * The Plane class is a mathematical object that defines a plane.
 */
class Plane
{
public:
    LIVRECORE_API Plane();

    /**
     * @param params normal vector and d value of plane. The normalization is run on the normal and value are
     * modified accordingly.
     */
    LIVRECORE_API Plane( const Vector4f& params );

    /**
     * Plane defined by implicit ax + by + cz + d = 0 equation.
     * @param a Plane equation parameter.
     * @param b Plane equation parameter.
     * @param c Plane equation parameter.
     * @param d Plane equation parameter.
     */
    LIVRECORE_API Plane( float a, float b, float c, float d );

    /**
     * Translates plane in the normal direction for a given distance.
     * @param distance parameter.
     * @return A new plane translated.
     */
    LIVRECORE_API Plane translate( float distance ) const;

    /**
     * @param pnt is the point to compute the distance.
     * @return The shortest distance of a point to the plane.
     */
    LIVRECORE_API float distance( const Vector3f& pnt ) const;

    /**
     * @return The d parameter of the plane ( distance to origin ).
     */
    LIVRECORE_API float getd( ) const;

    /**
     * Normalizes the plane normal, and changes the d value as well.
     */
    LIVRECORE_API void normalize( );

    /**
     * @return The normal of the plane.
     */
    LIVRECORE_API Vector3f getNormal( ) const;

    /**
     * @return The normal of the plane. Each value of the normal vector is pozitive.
     */
    LIVRECORE_API Vector3f getAbsNormal( ) const;

    /**
     * @return True if a box is completely under a plane.
     */
    LIVRECORE_API bool intersectOrUnder( const Boxf& bb ) const;

    /**
     * Sets the parameters of the plane. Normalizes the plane normal, and changes the d value as well.
     * @param a Plane normal x value.
     * @param b Plane normal y value.
     * @param c Plane normal z value.
     * @param d Plane normal d value.
     */
    LIVRECORE_API void set( const float a, const float b, const float c, const float d );

    /**
     * @return The plane parameters.
     */
    LIVRECORE_API Vector4f getPlaneCoeffs() const;

    /*
     * Near far points of AABB are identified according to plane
     */
    LIVRECORE_API void getNearFarPoints( const Boxf& bb, Vector3f& near, Vector3f& far ) const;

private:

    // Variables to define a Plane in 3D space a*x + b*y + c*z + d = 0
    float a_, b_, c_, d_;
};

}

#endif // _Plane_h_
