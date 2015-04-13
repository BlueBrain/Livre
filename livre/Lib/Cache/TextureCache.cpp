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

#include <livre/Lib/Cache/TextureCache.h>
#include <livre/Lib/Cache/RawDataObject.h>
#include <livre/Lib/Cache/TextureDataObject.h>
#include <livre/Lib/Cache/TextureObject.h>

#include <livre/core/Data/LODNode.h>
#include <livre/core/Data/VolumeDataSource.h>
#include <livre/core/Render/gl.h>
#include <livre/core/Render/Renderer.h>

namespace livre
{

TextureCache::TextureCache( const GLint internalTextureFormat )
    : texturePoolFactory_( internalTextureFormat )
{
    statisticsPtr_->setStatisticsName( "Texture cache statistics");
}

CacheObject *TextureCache::generateCacheObjectFromID_( const CacheId cacheID LB_UNUSED )
{
    return new TextureObject( TextureCachePtr( this, DontDeleteObject< TextureCache >() ) );
}

TextureObject& TextureCache::getNodeTexture( const CacheId cacheID  )
{
    if( cacheID == INVALID_CACHE_ID )
        return *TextureObject::getEmptyPtr();

    TextureObject* internalTexture = static_cast< TextureObject *>(
                getObjectFromCache_( cacheID ).get() );

    return *internalTexture;
}


TextureObject& TextureCache::getNodeTexture( const CacheId cacheID  ) const
{
    if( cacheID == INVALID_CACHE_ID )
        return *TextureObject::getEmptyPtr();

    TextureObject* internalTexture = static_cast< TextureObject *>( getObjectFromCache_( cacheID ).get() );

    return internalTexture != NULL ? *internalTexture : *TextureObject::getEmptyPtr();
}

TexturePoolPtr TextureCache::getTexturePool( const Vector3i& maxBlockSize,
                                             const GLenum format,
                                             const GLenum gpuDataType)
{
    return texturePoolFactory_.findTexturePool( maxBlockSize, format, gpuDataType );
}

}
