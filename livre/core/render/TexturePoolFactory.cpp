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

#include "TexturePoolFactory.h"

#include <livre/core/render/TexturePool.h>
#include <eq/gl.h>

namespace livre
{

TexturePoolFactory::TexturePoolFactory( const GLint internalFormat )
    : internalFormat_( internalFormat )
{
}

TexturePoolPtr TexturePoolFactory::findTexturePool( const Vector3i& maxBlockSize,
                                                    const GLenum format,
                                                    const GLenum gpuDataType )
{
    for( TexturePoolPtrVector::iterator it = texturePoolList_.begin();
         it != texturePoolList_.end(); ++it )
    {
        TexturePoolPtr pool = *it;
        if( maxBlockSize == pool->getMaxBlockSize() &&
           format == pool->getFormat() &&
            gpuDataType == pool->getGPUDataType() )
        {
            return pool;
        }
    }

    TexturePoolPtr pool(  new TexturePool( maxBlockSize,
                                           internalFormat_,
                                           format,
                                           gpuDataType ) );
    texturePoolList_.push_back( pool );
    return pool;
}

}
