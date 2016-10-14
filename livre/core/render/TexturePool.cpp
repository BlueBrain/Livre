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

#include <livre/core/data/DataSource.h>

#include <GL/glew.h>

namespace livre
{

TexturePool::TexturePool( const DataSource& dataSource, const size_t gpuCacheSize )
    : _maxBlockSize( dataSource.getVolumeInfo().maximumBlockSize )
    , _internalTextureFormat( 0 )
    , _format( 0 )
    , _textureType( 0 )
{
    if( dataSource.getVolumeInfo().compCount != 1 )
        LBTHROW( std::runtime_error( "Unsupported number of channels." ));

    switch( dataSource.getVolumeInfo().dataType )
    {
    case DT_UINT8:
        _internalTextureFormat = GL_R8UI;
        _format = GL_RED_INTEGER;
        _textureType = GL_UNSIGNED_BYTE;
    break;
    case DT_UINT16:
        _internalTextureFormat = GL_R16UI;
        _format = GL_RED_INTEGER;
        _textureType = GL_UNSIGNED_SHORT;
    break;
    case DT_UINT32:
        _internalTextureFormat = GL_R32UI;
        _format = GL_RED_INTEGER;
        _textureType = GL_UNSIGNED_INT;
    break;
    case DT_INT8:
        _internalTextureFormat = GL_R8I;
        _format = GL_RED_INTEGER;
        _textureType = GL_BYTE;
    break;
    case DT_INT16:
        _internalTextureFormat = GL_R16I;
        _format = GL_RED_INTEGER;
        _textureType = GL_SHORT;
    break;
    case DT_INT32:
        _internalTextureFormat = GL_R32I;
        _format = GL_RED_INTEGER;
        _textureType = GL_INT;
    break;
    case DT_FLOAT:
        _internalTextureFormat = GL_R32F;
        _format = GL_RED;
        _textureType = GL_FLOAT;
    break;
    case DT_UNDEFINED:
    default:
       LBTHROW( std::runtime_error( "Undefined data type" ));
    break;
    }

    const size_t blockMemSize = _maxBlockSize.product() *
                                dataSource.getVolumeInfo().getBytesPerVoxel() *
                                dataSource.getVolumeInfo().compCount;

    const size_t nBlockCount = gpuCacheSize / blockMemSize;

    std::vector< std::unique_ptr <TextureState > > states;
    for( size_t i = 0; i < nBlockCount; ++i )
        states.emplace_back( new TextureState( *this ));
    states.clear();
}

TexturePool::~TexturePool()
{}

void TexturePool::generateTexture( TextureState& textureState )
{
    ScopedLock lock( _mutex );
    LBASSERT( textureState.textureId == INVALID_TEXTURE_ID );
    if( !_textureStack.empty() )
    {
        textureState.textureId = _textureStack.back();
        _textureStack.pop_back();
    }
    else
    {
        glGenTextures( 1, &textureState.textureId );
        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
        textureState.bind();
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // Allocate a texture
        glTexImage3D( GL_TEXTURE_3D, 0, _internalTextureFormat,
                      _maxBlockSize[ 0 ], _maxBlockSize[ 1 ], _maxBlockSize[ 2 ], 0,
                      _format, _textureType, (GLvoid *)NULL );

        const GLenum glErr = glGetError();
        if( glErr != GL_NO_ERROR )
            LBERROR << "Error loading the texture into GPU, error number: " << glErr << std::endl;

    }
}

void TexturePool::releaseTexture( TextureState& textureState )
{
    ScopedLock lock( _mutex );
    LBASSERT( textureState.textureId );
    _textureStack.push_back( textureState.textureId );
}

}
