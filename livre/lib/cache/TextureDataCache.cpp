/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include <livre/lib/cache/TextureDataCache.h>
#include <livre/lib/cache/TextureDataObject.h>

#include <livre/core/data/LODNode.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/render/Renderer.h>

#include <eq/gl.h>

namespace livre
{

TextureDataCache::TextureDataCache( VolumeDataSourcePtr dataSource,
                                    const uint32_t textureType )
    : _dataSource( dataSource )
    , _textureType( textureType )
{
    _statistics->setStatisticsName( "Data cache CPU");
}

CacheObject* TextureDataCache::_generate( const CacheId& cacheId )
{
    if( cacheId == INVALID_CACHE_ID )
        return static_cast< CacheObject* >( TextureDataObject::getEmptyPtr( ));

    return new TextureDataObject( cacheId, _dataSource, _textureType );
}

TextureDataObject& TextureDataCache::getNodeTextureData( const CacheId& cacheId )
{
    if( cacheId == INVALID_CACHE_ID )
        return *TextureDataObject::getEmptyPtr();

    TextureDataObject* internalTextureData =
            static_cast< TextureDataObject *>( _get( cacheId ).get( ));

    return *internalTextureData;
}

TextureDataObject& TextureDataCache::getNodeTextureData( const CacheId& cacheId ) const
{
    if( cacheId == INVALID_CACHE_ID )
        return *TextureDataObject::getEmptyPtr();

    TextureDataObject* internalTextureData =
            static_cast< TextureDataObject *>( _get( cacheId ).get( ));

    return internalTextureData != NULL ? *internalTextureData : *TextureDataObject::getEmptyPtr();
}

VolumeDataSourcePtr TextureDataCache::getDataSource()
{
    return _dataSource;
}

}
