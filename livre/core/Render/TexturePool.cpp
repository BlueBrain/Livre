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

#include "TexturePool.h"

#include <livre/core/defines.h>
#include <livre/core/Render/GLContext.h>
#include <eq/gl.h>

namespace livre
{

#define glewGetContext() GLContext::glewGetContext()

TexturePool::TexturePool( const Vector3i& maxBlockSize,
                          const GLint internalFormat,
                          const GLenum format,
                          const GLenum gpuDataType )
    : maxBlockSize_( maxBlockSize )
    , internalFormat_( internalFormat )
    , format_( format )
    , gpuDataType_( gpuDataType )
{
}

void TexturePool::generateTexture( TextureStatePtr textureState )
{
    LBASSERT( textureState->textureId == INVALID_TEXTURE_ID );

    if( !textureStack_.empty() )
    {
        textureState->textureId = textureStack_.back();
        textureStack_.pop_back();
    }
    else
    {
        glGenTextures( 1, &textureState->textureId );
        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
        textureState->bind();
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // Allocate a texture
        glTexImage3D( GL_TEXTURE_3D, 0, internalFormat_,
                      maxBlockSize_[0], maxBlockSize_[1], maxBlockSize_[2], 0,
                      format_, gpuDataType_, (GLvoid *)NULL );

        const GLenum glErr = glGetError();
        if ( glErr != GL_NO_ERROR )
        {
            LBERROR << "Error loading the texture into GPU, error number : " << glErr  << std::endl;
        }
    }
}

void TexturePool::releaseTexture( TextureStatePtr textureState )
{
    LBASSERT( textureState->textureId );

    textureStack_.push_back( textureState->textureId );
    textureState->textureId = INVALID_TEXTURE_ID;
}

GLint TexturePool::getInternalFormat() const
{
    return internalFormat_;
}

GLenum TexturePool::getGPUDataType() const
{
    return gpuDataType_;
}

GLenum TexturePool::getFormat() const
{
    return format_;
}

const Vector3i& TexturePool::getMaxBlockSize( ) const
{
    return maxBlockSize_;
}

}
