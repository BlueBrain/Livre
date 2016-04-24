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

#ifndef _RenderBrick_h_
#define _RenderBrick_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/render/TextureState.h>

namespace livre
{

/**
 * The RenderBrick class, is used to keep the rendering information of an LOD block. It can also render the
 * faces of the block and can compute the screen coordinates with given parameters.
 */
class RenderBrick
{
public:
    /**
     * @param lodNodePtr livre::LODNode is used for spatial information of rendered block.
     * @param textureState TextureState is used to hold the information of OpenGL texture and coordinates.
     */
    LIVRECORE_API RenderBrick( const LODNode& lodNode,
                               const ConstTextureStatePtr& textureState );

    /**
     * @return Return LODNode.
     */
    const LODNode& getLODNode() const { return _lodNode; }

    /**
     * @return The information of OpenGL texture and coordinates.
     */
    ConstTextureStatePtr getTextureState() const { return _textureState; }

    /**
     * Renders the faces of a RenderBrick.
     * @param front Only draw front faces.
     * @param back Only draw back faces.
     */
    LIVRECORE_API void drawBrick( bool front, bool back ) const;

private:

    LODNode _lodNode;
    ConstTextureStatePtr _textureState;
};

}
#endif // _RenderBrick_h_
