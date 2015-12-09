/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <livre/lib/cache/TextureCache.h>
#include <livre/lib/cache/TextureDataObject.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/data/LODNode.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/render/Renderer.h>

#include <eq/gl.h>

namespace livre
{

TextureCache::TextureCache( size_t maxMem,
                            const GLint internalTextureFormat )
    : LRUCache( maxMem )
    , texturePoolFactory_( internalTextureFormat )
{
    statisticsPtr_->setStatisticsName( "Texture cache GPU");
}

CacheObject *TextureCache::generateCacheObjectFromID_( const CacheId& cacheId )
{
    return new TextureObject( cacheId,
                              TextureCachePtr( this, DontDeleteObject< TextureCache >() ) );
}

TextureObject& TextureCache::getNodeTexture( const CacheId& cacheId  )
{
    if( cacheId == INVALID_CACHE_ID )
        return *TextureObject::getEmptyPtr();

    TextureObject* internalTexture = static_cast< TextureObject *>(
                getObjectFromCache_( cacheId ).get() );

    return *internalTexture;
}


TextureObject& TextureCache::getNodeTexture( const CacheId& cacheId  ) const
{
    if( cacheId == INVALID_CACHE_ID )
        return *TextureObject::getEmptyPtr();

    TextureObject* internalTexture =
            static_cast< TextureObject *>( getObjectFromCache_( cacheId ).get() );

    return internalTexture != NULL ? *internalTexture : *TextureObject::getEmptyPtr();
}

TexturePoolPtr TextureCache::getTexturePool( const Vector3i& maxBlockSize,
                                             const GLenum format,
                                             const GLenum gpuDataType)
{
    return texturePoolFactory_.findTexturePool( maxBlockSize, format, gpuDataType );
}

}
