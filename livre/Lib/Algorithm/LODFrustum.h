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

#ifndef _LODFrustum_h_
#define _LODFrustum_h_

#include <livre/core/Render/Frustum.h>
#include <livre/Lib/types.h>

namespace livre
{

/** Special frustum for LOD segmentation based on the quality parameters */

class LODFrustum
{
public:

    /** Constructs an empty LOD frustum. */
    LODFrustum( );

    /** Constructs an LOD frustum based on given parameters.
     * @param frustum Base view-frustum to segment.
     * @param screenSpaceError Selects level of detail according to given screen space error.
     * @param screenHeight The height of rendering screen.
     * @param worldSpacePerVoxel World space per pixel in the highest resolution ( 1.0 / max volume dim ).
     * @param nodeWorldSize Node world size in the highest resolution.
     * @param nbLODLevels The depth of the LOD Tree.
     * @param expandDistances If needed the planes of view-frustum can be translated by given distances.
     */
    LODFrustum( const Frustum& frustum,
                const float screenSpaceError,
                const uint32_t screenHeight,
                const float worldSpacePerVoxel,
                const float nodeWorldSize,
                const uint32_t nbLODLevels,
                const FloatVector& expandDistances );

    /**
     * @return The frustum that LOD frustum is based on.
     */
    const Frustum& getFrustum( ) const;

    /**
     * Checks whether the given AABB box is in segmented level.
     * @param worldBox The AABB to test in world coordinates.
     * @param lodLevel The LOD level segment to to test.
     * @return True if box is in the given LOD level.
     */
    bool boxInSubFrustum( const Boxf &worldBox, const int32_t lodLevel ) const;

    /**
     * Checks whether the given AABB box is in the Frustum.
     * @param worldBox The AABB to test in world coordinates.
     * @return True if box is in the expanded frustum.
     */
    bool boxInFrustum( const Boxf &worldBox ) const;

    /**
     * @return The total number of LOD nodes in the frustum.
     */
    uint32_t getMaxNumberOfNodes( ) const;

    /**
     * @return Get current expansion distances
     */
    const FloatVector& getExpandDistances( ) const;

    /**
     * @param planeId The plane id for the corresponding expansion distance.
     * @return The current expansion distance based of planeId.
     */
    float getExpandDistance( const PlaneId planeId ) const;

    /**
     * @param worldPoint is the point in world space.
     * @return the lod for a given world point.
     */
    uint32_t getLODForPoint( const Vector3f& worldPoint ) const;

private:

    Vector3f computeCorner_( const PlaneId planeId0,
                             const PlaneId planeId1,
                             const PlaneId planeId2) const;
    void computeCorners_();

    void initializeWorldPlanes_( );
    void computeBaseFrustumLODLimits_();
    void computeFrustumLODLimits_( );

    FloatVector baseFrustumLODLowerLimits_;
    FloatVector baseFrustumLODUpperLimits_;
    FloatVector frustumLODUpperLimits_;
    FloatVector frustumLODLowerLimits_;
    FloatVector frustumMaxNbLODNodes_;

    Frustum frustum_;
    float screenSpaceError_;
    uint32_t screenHeight_;
    float worldSpacePerVoxel_;
    float nodeWorldSize_;
    uint32_t nbLODLevels_;
    uint32_t totalNumberOfNodes_;
    FloatVector expandDistances_;

    Plane mvPlanes_[ 6 ];

    // Modelview space
    Vector3f nTopLeft_;
    Vector3f nTopRight_;
    Vector3f nBottomLeft_;
    Vector3f nBottomRight_;
    Vector3f fTopLeft_;
    Vector3f fTopRight_;
    Vector3f fBottomLeft_;
    Vector3f fBottomRight_;

};

}

#endif // _LODFrustum_h_
