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

    bool operator()( const RenderBrick& rb1,
                     const RenderBrick& rb2 )
    {
        const float distance1 = ( _frustum.getMVMatrix() *
                                  rb1.getLODNode().getWorldBox().getCenter() ).length();
        const float distance2 = ( _frustum.getMVMatrix() *
                                  rb2.getLODNode().getWorldBox().getCenter() ).length();
        return  distance1 < distance2;
    }
    const Frustum& _frustum;
};

Renderer::~Renderer()
{
}

RenderBricks Renderer::_order( const RenderBricks& bricks,
                               const Frustum& frustum ) const
{
    RenderBricks rbs = bricks;
    DistanceOperator distanceOp( frustum );
    std::sort( rbs.begin(), rbs.end(), distanceOp );
    return rbs;
}

void Renderer::_onFrameRender( const Frustum& frustum,
                               const PixelViewport& view,
                               const RenderBricks& bricks )
{
    for( const RenderBrick& brick: bricks )
        _renderBrick( frustum, view, brick );
}

void Renderer::render( const Frustum& frustum,
                       const PixelViewport& view,
                       const RenderBricks& bricks )
{
    const RenderBricks& ordered = _order( bricks, frustum );
    _onFrameStart( frustum, view, ordered );
    _onFrameRender( frustum, view, ordered );
    _onFrameEnd( frustum, view, ordered );
}

}
