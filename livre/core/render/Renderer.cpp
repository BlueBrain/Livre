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

#include "Renderer.h"

#include <livre/core/render/TexturePoolFactory.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/RenderBrick.h>
#include <livre/core/data/LODNode.h>
#include <eq/gl.h>

namespace livre
{

// Sort helper funtion for sorting the textures with their distances to viewpoint
struct DistanceOperator
{
    explicit DistanceOperator( const Frustum& frustum ) : frustum_( frustum ) { }
    bool operator()( const RenderBrickPtr& rb1,
                     const RenderBrickPtr& rb2 )
    {
        const float distance1 = ( frustum_.getModelViewMatrix() *
                                  rb1->getLODNode().getWorldBox().getCenter() ).length();
        const float distance2 = ( frustum_.getModelViewMatrix() *
                                  rb2->getLODNode().getWorldBox().getCenter() ).length();
        return  distance1 < distance2;
    }
    const Frustum& frustum_;
};

Renderer::Renderer( const uint32_t nComponents,
                    const GLenum gpuDataType,
                    const GLint internalFormat )
    : gpuDataType_( gpuDataType ),
      internalFormat_( internalFormat )
{

    switch( nComponents )
    {
        case 1:
            format_ = GL_RED;
            break;
        case 3:
            format_ = GL_RGB;
            break;
        default:
            format_ = GL_RED;
            break;
    }
}

Renderer::~Renderer()
{
}

GLint Renderer::getInternalFormat() const
{
    return internalFormat_;
}

GLenum Renderer::getGPUDataType() const
{
    return gpuDataType_;
}

GLenum Renderer::getFormat() const
{
    return format_;
}

void Renderer::order_( RenderBricks &bricks, const Frustum &frustum ) const
{
    DistanceOperator distanceOp( frustum );
    std::sort( bricks.begin(), bricks.end(), distanceOp );
}


void Renderer::onFrameRender_( const GLWidget& glWidget, const View& view,
                               const Frustum& frustum,
                               const RenderBricks &bricks )
{
    BOOST_FOREACH( const RenderBrickPtr& brick, bricks )
        renderBrick_( glWidget, view, frustum, *brick );
}

void Renderer::render( const GLWidget& glWidget,
                       const View& view,
                       const Frustum& frustum,
                       const RenderBricks& brickList )
{
    RenderBricks bricks = brickList;
    order_( bricks, frustum );
    onFrameStart_( glWidget, view, frustum, bricks );
    onFrameRender_( glWidget, view, frustum, bricks );
    onFrameEnd_( glWidget, view, frustum, bricks );
}

}
