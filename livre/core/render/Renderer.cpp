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

#include <livre/core/render/Frustum.h>
#include <livre/core/render/View.h>
#include <livre/core/render/RenderBrick.h>
#include <livre/core/data/LODNode.h>
#include <eq/gl.h>

namespace livre
{

// Sort helper funtion for sorting the textures with their distances to viewpoint
struct DistanceOperator
{
    explicit DistanceOperator( const Frustum& frustum )
        : _frustum( frustum )
    { }

    bool operator()( const RenderBrickPtr& rb1,
                     const RenderBrickPtr& rb2 )
    {
        const float distance1 = ( _frustum.getModelViewMatrix() *
                                  rb1->getLODNode().getWorldBox().getCenter() ).length();
        const float distance2 = ( _frustum.getModelViewMatrix() *
                                  rb2->getLODNode().getWorldBox().getCenter() ).length();
        return  distance1 < distance2;
    }
    const Frustum& _frustum;
};

Renderer::Renderer( const uint32_t nComponents,
                    const GLenum gpuDataType,
                    const GLint internalFormat )
    : _gpuDataType( gpuDataType ),
      _internalFormat( internalFormat )
{

    switch( nComponents )
    {
        case 1:
            _format = GL_RED;
            break;
        case 3:
            _format = GL_RGB;
            break;
        default:
            LBTHROW( std::runtime_error( "Unsupported texture format" ));
    }
}

Renderer::~Renderer()
{
}

GLint Renderer::getInternalFormat() const
{
    return _internalFormat;
}

GLenum Renderer::getGPUDataType() const
{
    return _gpuDataType;
}

GLenum Renderer::getFormat() const
{
    return _format;
}

void Renderer::order_( RenderBricks &bricks, const Frustum &frustum ) const
{
    DistanceOperator distanceOp( frustum );
    std::sort( bricks.begin(), bricks.end(), distanceOp );
}


void Renderer::_onFrameRender( const GLWidget& glWidget,
                               const View& view,
                               const RenderBricks &bricks )
{
    for( const RenderBrickPtr& brick: bricks )
        _renderBrick( glWidget, view, *brick );
}

void Renderer::render( const GLWidget& glWidget,
                       const View& view,
                       const RenderBricks& brickList )
{
    RenderBricks bricks = brickList;
    order_( bricks, view.getFrustum( ));
    _onFrameStart( glWidget, view, bricks );
    _onFrameRender( glWidget, view, bricks );
    _onFrameEnd( glWidget, view, bricks );
}

}
