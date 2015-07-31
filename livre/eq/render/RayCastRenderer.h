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

#ifndef _RayCastRenderer_h_
#define _RayCastRenderer_h_

#include <livre/eq/types.h>
#include <livre/eq/eqTypes.h>
#include <livre/core/render/Renderer.h>
#include <livre/core/render/TransferFunction1D.h>

namespace livre
{

/**
 * The RayCastRenderer class implements a single-pass ray caster.
 */
class RayCastRenderer : public Renderer
{
public:

    /**
     * @param samples Number of samples per ray.
     * @param componentCount Component count of rendering.
     * @param gpuDataType Data type of the texture data source.
     * @param internalFormat Internal format of the texture in GPU memory.
     */
    RayCastRenderer( const uint32_t samples, const uint32_t componentCount,
                     const uint32_t gpuDataType, const int32_t internalFormat );
    ~RayCastRenderer();

    /**
     * Modifies the transfer function.
     * @param transferFunction Transfer function.
     */
    void initTransferFunction( const TransferFunction1D< uint8_t >& transferFunction );

private:
    void onFrameStart_( const GLWidget& glWidget,
                        const View& view,
                        const Frustum &frustum,
                        const RenderBricks &brickList );

    void renderBrick_( const GLWidget& glWidget,
                       const View& view,
                       const Frustum &frustum,
                       const RenderBrick& renderBrick );

    bool readFromFrameBuffer_( const GLWidget& glWidget,
                               const View& view,
                               const Frustum &frustum,
                               const RenderBrick& renderBrick,
                               Vector2i& screenPos );


    EqTexturePtr _framebufferTexture;
    GLSLShadersPtr _shaders;
    uint32_t _nSamples;
    uint32_t _transferFunctionTexture;
    std::vector< uint32_t > _usedTextures[2]; // last, current frame
};

}

#endif // _RayCastRenderer_h_
