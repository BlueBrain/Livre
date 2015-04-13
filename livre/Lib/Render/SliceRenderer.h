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

#ifndef _SliceRenderer_h_
#define _SliceRenderer_h_

#include <livre/Lib/types.h>
#include <livre/core/Render/Renderer.h>

namespace livre
{

/**
 * The SliceRenderer class implements the sliced texture rendering algorithm.
 */
class SliceRenderer: public Renderer
{

public:

    /**
     * @param samples Number of samples per ray.
     * @param compCount Component count of rendering.
     * @param gpuDataType Data type of the texture data source.
     * @param internalFormat Internal format of the texture in GPU memory.
     */
    SliceRenderer( const uint32_t samples, const uint32_t compCount,
                   const uint32_t gpuDataType, const int32_t internalFormat );

private:

    void renderBrick_( const GLWidget& glWidget,
                       const View& view,
                       const Frustum &frustum,
                       const RenderBrick& renderBrick );

    int32_t generateAxisAlignedSlices_( const RenderBrick& renderBrick,
                                      const std::vector<Vector4f> &corners,
                                      const std::vector< Vector4f >& rotatedCorners,
                                      int32_t order[ 6 ],
                                      Vector3f textureCoords[ 6 ],
                                      Vector3f vertices[ 6 ],
                                      Vector3f edges[ 12 ],
                                      const float snSp,
                                      const float snP0[12],
                                      const float snEdge[12] );

    int32_t intersect_( const RenderBrick& renderBrick,
                      const std::vector<Vector4f> &corners,
                      const std::vector< Vector4f >& rotatedCorners,
                      Vector3f verts[ 6 ],
                      Vector3f tverts[ 6 ],
                      Vector3f rverts[ 6 ],
                      Vector3f edges[ 12 ],
                      const float snSp,
                      const float snP0[12],
                      const float snEdge[12] );

    void findVertexOrder_( const Vector3f rotatedVerts[],
                           int32_t order[ 6 ],
                           int32_t degree );

    void getCorners_( const Boxf& box,
                      std::vector< Vector4f >&corners );

    void getTransformedCorners_( const std::vector< Vector4f >&corners,
                                 const Matrix4f &transform,
                                 std::vector< Vector4f >& transformedCorners );

    uint32_t samples_;
};

}

#endif // _SliceRenderer_h_
