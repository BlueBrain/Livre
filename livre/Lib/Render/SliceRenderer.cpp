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

#include "SliceRenderer.h"

#include <livre/core/Data/LODNode.h>
#include <livre/core/Render/Frustum.h>
#include <livre/core/Render/RenderBrick.h>
#include <livre/core/Render/gl.h>

namespace livre
{

SliceRenderer::SliceRenderer( const uint32_t samples,
                              const uint32_t compCount,
                              const GLenum gpuDataType /* = GL_UNSIGNED_BYTE */,
                              const GLint internalFormat /* = GL_LUMINANCE8 */)
    :
      Renderer( compCount,
                gpuDataType,
                internalFormat ),
      samples_( samples )
{
}

void SliceRenderer::renderBrick_( const GLWidget& glWidget LB_UNUSED,
                                  const View& view LB_UNUSED,
                                  const Frustum &frustum,
                                  const RenderBrick& renderBrick )
{
     // Inverse modelview matrix for world normal of slice
    const Matrix4f& mv = frustum.getModelViewMatrix();
    const Vector3f sliceNormal( mv(2, 0), mv(2, 1), mv(2, 2) );

    const ConstLODNodePtr& lodNodePtr = renderBrick.getLODNode();
    const Boxf& worldBox = lodNodePtr->getWorldBox();

    std::vector< Vector4f > corners;
    corners.resize( 8 );
    getCorners_( worldBox, corners );

    std::vector< Vector4f > rotatedCorners;
    rotatedCorners.resize( 8 );
    for( uint32_t i = 0; i < 8; ++i )
    {
        rotatedCorners[ i ] = mv * corners[ i ];
    }

    // Find minimum, maximum according to depth
    int32_t minIndex = 0;
    int32_t maxIndex = 0;
    for( int32_t i = 0; i < 8; ++i )
    {
        if (rotatedCorners[ minIndex ][ 2 ] > rotatedCorners[ i ][ 2 ] ) minIndex = i;
        if (rotatedCorners[ maxIndex ][ 2 ] < rotatedCorners[ i ][ 2 ] ) maxIndex = i;
    }

    float samples = samples_ /  ( 1 <<  lodNodePtr->getRefLevel() );

    Vector3f slicePoint32 = corners[ minIndex ];

    const float delta = ( rotatedCorners[ maxIndex ][ 2 ] - rotatedCorners[ minIndex ][ 2 ] ) / samples;

    const Vector3f sliceDelta = sliceNormal * delta;

    Vector3f edges[ 12 ];

    //       6*--------*7
    //       /|       /|
    //      / |      / |
    //     /  |     /  |
    //    /  4*----/---*5
    //  2*--------*3  /
    //   |  /     |  /
    //   | /      | /
    //   |/       |/
    //  0*--------*1
    //

    const int32_t edgesArr[12][3] =
    {
         {0, 1, 0}, // front bottom edge x
         {0, 2, 1}, // front left edge   y
         {1, 3, 1}, // front right edge  y
         {4, 0, 2}, // left bottom edge  z
         {1, 5, 2}, // right bottom edge z
         {2, 3, 0}, // front top edge    x
         {4, 5, 0}, // back bottom edge  x
         {4, 6, 1}, // back left edge    y
         {5, 7, 1}, // back right edge   y
         {6, 7, 0}, // back top edge     x
         {2, 6, 2}, // left top edge     z
         {3, 7, 2}  // right top edge    z
    };

    // t = ( sliceNormal.dot( slicePoint32 - *p0 ) / sliceNormal.dot( diff ) );
    // t = (sn.sp - sn.p0) / sn.diff, sn.sp const per sample, 1 / sn.Edge const per edge, sn.sp const per edge
    float snP0[ 12 ];
    float snEdge[ 12 ];
    for (int32_t i = 0; i < 12; i++ )
    {
        const Vector3f* p0 = ( const Vector3f* )&corners[ edgesArr[ i ][ 0 ] ];
        const Vector3f* p1 = ( const Vector3f* )&corners[ edgesArr[ i ][ 1 ] ];
        edges[ i ] = *p1 - *p0;
        snP0[ i ] = sliceNormal.dot( *p0 );
        snEdge[ i ] = 1.0f / sliceNormal.dot( edges[ i ] );
    }

    for( uint32_t i = 0 ; i <= samples_; ++i )
    {
        int32_t order[ 6 ] = { 0, 1, 2, 3, 4, 5 };
        Vector3f vertices[ 6 ];
        Vector3f textureCoords[ 6 ];
        const float snSp = sliceNormal.dot( slicePoint32 );
        const int32_t size = generateAxisAlignedSlices_( renderBrick,
                                                     corners,
                                                     rotatedCorners,
                                                     order,
                                                     textureCoords,
                                                     vertices,
                                                     edges,
                                                     snSp,
                                                     snP0,
                                                     snEdge );
        glDisable(GL_CULL_FACE);
        glBegin(GL_POLYGON);
        {
            for(int32_t j=0; j< size; ++j)
            {
                glTexCoord3f( textureCoords[ order[ j ] ][ 0 ],
                              textureCoords[ order[ j ] ][ 1 ],
                              textureCoords[ order[ j ] ][ 2 ] );

                glVertex3f( vertices[ order[ j ] ][ 0 ],
                            vertices[ order[ j ] ][ 1 ],
                            vertices[ order[ j ] ][ 2 ] );
            }
        }
        glEnd();
        glEnable(GL_CULL_FACE);
        slicePoint32 += sliceDelta;
    }

}

int32_t SliceRenderer::generateAxisAlignedSlices_( const RenderBrick& renderBrick,
                                               const std::vector<Vector4f> &corners,
                                               const std::vector<Vector4f> &rotatedCorners,
                                               int32_t order[ 6 ],
                                               Vector3f textureCoords[ 6 ],
                                               Vector3f vertices[ 6 ],
                                               Vector3f edges[ 12 ],
                                               const float snSp,
                                               const float snP0[12],
                                               const float snEdge[12] )
{
    //
    // Intersect the slice plane with the bounding boxes
    //
    Vector3f rotatedVertices[ 6 ];
    const int32_t size = intersect_( renderBrick,
                                 corners,
                                 rotatedCorners,
                                 vertices,
                                 textureCoords,
                                 rotatedVertices,
                                 edges,
                                 snSp,
                                 snP0,
                                 snEdge );
    //
    // Calculate the convex hull of the vertices (world coordinates)
    //
    findVertexOrder_( rotatedVertices,
                      order,
                      size );

    return size;
}

int32_t SliceRenderer::intersect_( const RenderBrick& renderBrick,
                               const std::vector< Vector4f > &corners,
                               const std::vector< Vector4f >& rotatedCorners,
                               Vector3f verts[ 6 ],
                               Vector3f tverts[ 6 ],
                               Vector3f rverts[ 6 ],
                               Vector3f edges[ 12 ],
                               const float snSp,
                               const float snP0[12],
                               const float snEdge[12] )
{
    //       6*--------*7
    //       /|       /|
    //      / |      / |
    //     /  |     /  |
    //    /  4*----/---*5
    //  2*--------*3  /
    //   |  /     |  /
    //   | /      | /
    //   |/       |/
    //  0*--------*1

    const int32_t edgesArr[12][3] =
    {
         {0, 1, 0}, // front bottom edge x
         {0, 2, 1}, // front left edge   y
         {1, 3, 1}, // front right edge  y
         {4, 0, 2}, // left bottom edge  z
         {1, 5, 2}, // right bottom edge z
         {2, 3, 0}, // front top edge    x
         {4, 5, 0}, // back bottom edge  x
         {4, 6, 1}, // back left edge    y
         {5, 7, 1}, // back right edge   y
         {6, 7, 0}, // back top edge     x
         {2, 6, 2}, // left top edge     z
         {3, 7, 2}  // right top edge    z
    };

    const ConstLODNodePtr& lodNodePtr = renderBrick.getLODNode();
    const Vector3f& textureSize = renderBrick.getTextureState()->textureSize;

    int32_t intersections = 0;

    for (int32_t i = 0; i < 12; i++ )
    {
        const float t = ( snSp - snP0[ i ] ) * snEdge[ i ];

        if( (t >= 0.0f ) && ( t <= 1.0f ) )
        {
            const Vector4f& r0 = rotatedCorners[ edgesArr[ i ][ 0 ] ];
            const Vector4f& r1 = rotatedCorners[ edgesArr[ i ][ 1 ] ];

            // Compute the line intersection
            const Vector3f* p0 = ( const Vector3f* )&corners[ edgesArr[ i ][ 0 ] ];
            verts[ intersections ] = *p0 + ( edges[ i ] ) * t;

            // Compute the texture interseciton
            tverts[ intersections ] = verts[ intersections ] - lodNodePtr->getWorldBox().getMin();
            tverts[ intersections ] = tverts[ intersections ] / lodNodePtr->getWorldBox().getDimension();
            tverts[ intersections ] = tverts[ intersections ] * textureSize +
                    renderBrick.getTextureState()->textureCoordsMin;

            // Compute view coordinate intersection
            rverts[ intersections ] = r0 + ( r1 - r0 ) * t;
            intersections++;
            if( intersections == 6 )
                break;
        }
    }

    return intersections;
}

void SliceRenderer::findVertexOrder_( const Vector3f rotatedVerts[ 6 ],
                                      int32_t order[ 6 ],
                                      int32_t degree )
{
    //
    // Find the center of the polygon
    //
    float centroid[ 2 ] = { 0.0, 0.0 };

    for( int32_t i=0; i < degree; ++i)
    {
        centroid[0] += rotatedVerts[ i ][ 0 ];
        centroid[1] += rotatedVerts[ i ][ 1 ];
    }

    centroid[ 0 ] /= degree;
    centroid[ 1 ] /= degree;

    //
    // Compute the angles from the centroid
    //
    float angle[6];

    for(int32_t i=0; i< degree; ++i)
    {
        const float dx = rotatedVerts[ i ][ 0 ] - centroid[ 0 ];
        const float dy = rotatedVerts[ i ][ 1 ] - centroid[ 1 ];

        angle[ i ] = dy / ( fabs( dx ) + fabs( dy ) );

        if( dx < 0.0 )  // quadrants 2&3
        {
            angle[ i ] = 2.0 - angle[ i ];
        }
        else if( dy < 0.0 ) // quadrant 4
        {
            angle[ i ] = 4.0 + angle[ i ];
        }
    }

    //
    // Sort the angles (bubble sort)
    //
    for( int32_t i=0; i < degree; ++i )
    {
        for( int32_t j = i+1; j < degree; ++j )
        {
            if( angle[ j ] < angle[ i ] )
            {
                const float tmpd = angle[ i ];
                angle[ i ] = angle[ j ];
                angle[ j ] = tmpd;

                const int32_t tmpi = order[i];
                order[ i ] = order[ j ];
                order[ j ] = tmpi;
            }
        }
    }
}

void SliceRenderer::getCorners_( const Boxf &box,
                                 std::vector<Vector4f> &corners )
{
    const Vector3f& min = box.getMin();
    const Vector3f& max = box.getMax();

    corners[ 0 ].set( min[0], min[1], min[2], 1.0 );
    corners[ 1 ].set( max[0], min[1], min[2], 1.0 );
    corners[ 2 ].set( min[0], max[1], min[2], 1.0 );
    corners[ 3 ].set( max[0], max[1], min[2], 1.0 );
    corners[ 4 ].set( min[0], min[1], max[2], 1.0 );
    corners[ 5 ].set( max[0], min[1], max[2], 1.0 );
    corners[ 6 ].set( min[0], max[1], max[2], 1.0 );
    corners[ 7 ].set( max[0], max[1], max[2], 1.0 );
}

}
