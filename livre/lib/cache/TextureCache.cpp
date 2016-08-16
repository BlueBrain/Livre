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
#include <livre/lib/cache/DataCache.h>
#include <livre/lib/cache/DataObject.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/data/LODNode.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/render/Renderer.h>
#include <livre/core/cache/CacheStatistics.h>

#include <eq/gl.h>

namespace livre
{
struct TextureCache::Impl
{
    Impl( DataCache& dataCache )
        : _dataCache( dataCache )
    {
        const DataSource& dataSource = _dataCache.getDataSource();
        const VolumeInformation& info = dataSource.getVolumeInfo();

        if( info.compCount != 1 )
            LBTHROW( std::runtime_error( "Unsupported number of channels." ));

        uint32_t internalTextureFormat;
        uint32_t format;
        switch( dataSource.getVolumeInfo().dataType )
        {
            case DT_UINT8:
                internalTextureFormat = GL_R8UI;
                format = GL_RED_INTEGER;
                _textureType = GL_UNSIGNED_BYTE;
            break;
            case DT_UINT16:
                internalTextureFormat = GL_R16UI;
                format = GL_RED_INTEGER;
                _textureType = GL_UNSIGNED_SHORT;
            break;
            case DT_UINT32:
                internalTextureFormat = GL_R32UI;
                format = GL_RED_INTEGER;
                _textureType = GL_UNSIGNED_INT;
            break;
            case DT_INT8:
                internalTextureFormat = GL_R8I;
                format = GL_RED_INTEGER;
                _textureType = GL_BYTE;
            break;
            case DT_INT16:
                internalTextureFormat = GL_R16I;
                format = GL_RED_INTEGER;
                _textureType = GL_SHORT;
            break;
            case DT_INT32:
                internalTextureFormat = GL_R32I;
                format = GL_RED_INTEGER;
                _textureType = GL_INT;
            break;
            case DT_FLOAT:
                internalTextureFormat = GL_R32F;
                format = GL_RED;
                _textureType = GL_FLOAT;
            break;
            case DT_UNDEFINED:
            default:
                LBTHROW( std::runtime_error( "Undefined data type" ));
            break;
        }

        _texturePool.reset( new TexturePool( info.maximumBlockSize,
                                             internalTextureFormat,
                                             format,
                                             _textureType ));
    }

    CacheObject* generate( const CacheId& cacheId, TextureCache& cache )
    {
        return new TextureObject( cacheId, cache );
    }

    DataCache& _dataCache;
    std::unique_ptr< TexturePool > _texturePool;
    uint32_t _textureType;
};

TextureCache::TextureCache( DataCache& dataCache,
                            const size_t maxMemBytes )
    : Cache( "Texture cache GPU", maxMemBytes )
    , _impl( new Impl( dataCache ))
{}

CacheObject* TextureCache::_generate( const CacheId& cacheId )
{
    return _impl->generate( cacheId, *this );
}

TextureCache::~TextureCache()
{
    _unloadAll();
}

uint32_t TextureCache::getTextureType() const
{
    return _impl->_textureType;
}

TexturePool& TextureCache::getTexturePool() const
{
    return *_impl->_texturePool.get();
}

DataCache& TextureCache::getDataCache()
{
    return _impl->_dataCache;
}

const DataCache& TextureCache::getDataCache() const
{
    return _impl->_dataCache;
}

}
