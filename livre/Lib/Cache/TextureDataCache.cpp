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

#include <livre/Lib/Cache/RawDataObject.h>
#include <livre/Lib/Cache/TextureDataCache.h>
#include <livre/Lib/Cache/TextureDataObject.h>

#include <livre/core/Data/LODNode.h>
#include <livre/core/Data/VolumeDataSource.h>
#include <livre/core/Render/gl.h>
#include <livre/core/Render/Renderer.h>

namespace livre
{

TextureDataCache::TextureDataCache( const GLenum type )
    : type_( type )
{
    statisticsPtr_->setStatisticsName( "Texture data cache statistics");
}

CacheObject *TextureDataCache::generateCacheObjectFromID_( const CacheId cacheID LB_UNUSED )
{
    return new TextureDataObject( type_ );
}

TextureDataObject &TextureDataCache::getNodeTextureData( const CacheId cacheId )
{
    if( cacheId == INVALID_CACHE_ID )
        return *TextureDataObject::getEmptyPtr();

    TextureDataObject* internalTextureData =
            static_cast< TextureDataObject *>( getObjectFromCache_( cacheId ).get( ) );

    return *internalTextureData;
}

TextureDataObject &TextureDataCache::getNodeTextureData( const CacheId cacheId ) const
{
    if( cacheId == INVALID_CACHE_ID )
        return *TextureDataObject::getEmptyPtr();

    TextureDataObject* internalTextureData =
            static_cast< TextureDataObject *>( getObjectFromCache_( cacheId ).get( ) );

    return internalTextureData != NULL ? *internalTextureData : *TextureDataObject::getEmptyPtr();
}

}
