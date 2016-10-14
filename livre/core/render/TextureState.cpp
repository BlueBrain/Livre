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

#include "TextureState.h"

#include <livre/core/render/TexturePool.h>
#include <livre/core/defines.h>


namespace livre
{


TextureState::TextureState( TexturePool& texturePool )
    : textureCoordsMin( 0.0f )
    , textureCoordsMax( 0.0f )
    , textureSize( 0.0f )
    , textureId( INVALID_TEXTURE_ID )
    , _texturePool( texturePool )
{
    _texturePool.generateTexture( *this );
}

TextureState::~TextureState()
{
    _texturePool.releaseTexture( *this );
}

void TextureState::bind() const
{
    LBASSERTINFO( textureId != INVALID_TEXTURE_ID,
                  "GL error: " << glGetError( ));
    if( textureId == INVALID_TEXTURE_ID )
        return;

    glBindTexture( GL_TEXTURE_3D, textureId );
    const GLenum glErr = glGetError();
    if ( glErr != GL_NO_ERROR )
        LBERROR << "Error binding texture: "  << glErr << std::endl;
}

}
