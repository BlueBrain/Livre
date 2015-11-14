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
#include <livre/core/render/GLContext.h>
#include <eq/gl.h>

namespace livre
{

#define glewGetContext() GLContext::glewGetContext()


struct TexturePool::Impl
{
    Impl( const Vector3i& maxBlockSize,
          const int internalFormat,
          const uint32_t format,
          const uint32_t gpuDataType )
        : _maxBlockSize( maxBlockSize )
        , _internalFormat( internalFormat )
        , _format( format )
        , _gpuDataType( gpuDataType )
    {}

    void generateTexture( TextureStatePtr textureState )
    {
        LBASSERT( textureState->textureId == INVALID_TEXTURE_ID );

        if( !_texturePool.empty() )
            textureState->textureId = _texturePool.pop();
        else
        {
            glGenTextures( 1, &textureState->textureId );
            glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
            textureState->bind();
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            // Allocate a texture
            glTexImage3D( GL_TEXTURE_3D, 0,
                          _internalFormat,
                          _maxBlockSize[0],
                          _maxBlockSize[1],
                          _maxBlockSize[2], 0,
                          _format, _gpuDataType, (GLvoid *)NULL );

            const GLenum glErr = glGetError();
            if ( glErr != GL_NO_ERROR )
            {
                LBERROR << "Error loading the texture into GPU, error number : " << glErr  << std::endl;
            }
        }
    }

    void releaseTexture( TextureStatePtr textureState )
    {
        LBASSERT( textureState->textureId );

        _texturePool.push( textureState->textureId );
        textureState->textureId = INVALID_TEXTURE_ID;
    }

    lunchbox::MTQueue< uint32_t > _texturePool;
    const Vector3i _maxBlockSize;
    const int32_t _internalFormat;
    const uint32_t _format;
    const uint32_t _gpuDataType;
};

TexturePool::TexturePool( const Vector3i& maxBlockSize,
                          const GLint internalFormat,
                          const GLenum format,
                          const GLenum gpuDataType )
    : _impl( new TexturePool::Impl( maxBlockSize,
                                    internalFormat,
                                    format,
                                    gpuDataType ))
{
}


TexturePool::~TexturePool()
{
    
}

void TexturePool::generateTexture( TextureStatePtr textureState )
{
    _impl->generateTexture( textureState );
}

void TexturePool::releaseTexture( TextureStatePtr textureState )
{
     _impl->releaseTexture( textureState );
}

GLint TexturePool::getInternalFormat() const
{
    return _impl->_internalFormat;
}

GLenum TexturePool::getGPUDataType() const
{
    return _impl->_gpuDataType;
}

GLenum TexturePool::getFormat() const
{
    return _impl->_format;
}

const Vector3i& TexturePool::getMaxBlockSize( ) const
{
    return _impl->_maxBlockSize;
}

}
