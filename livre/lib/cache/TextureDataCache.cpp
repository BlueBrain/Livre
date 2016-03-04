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
#include <livre/core/data/DataSource.h>
#include <livre/core/render/Renderer.h>
#include <livre/core/cache/CacheStatistics.h>

#include <eq/gl.h>

namespace livre
{

struct TextureDataCache::Impl
{
public:
    Impl( DataSource& dataSource,
          const uint32_t textureType  )
        : _dataSource( dataSource )
        , _textureType( textureType )
    {}

    CacheObject* generate( const CacheId& cacheId, TextureDataCache& cache )
    {
        return new TextureDataObject( cacheId, cache );
    }

    DataSource& _dataSource;
    const uint32_t _textureType;
};

TextureDataCache::TextureDataCache( const size_t maxMemBytes,
                                    DataSource& dataSource,
                                    const uint32_t textureType )
    : Cache( "Data cache CPU", maxMemBytes )
    , _impl( new Impl( dataSource, textureType ))
{}

CacheObject* TextureDataCache::_generate( const CacheId& cacheId )
{
    return _impl->generate( cacheId, *this );
}

TextureDataCache::~TextureDataCache()
{
    _unloadAll();
}

DataSource& TextureDataCache::getDataSource()
{
    return _impl->_dataSource;
}

const DataSource& TextureDataCache::getDataSource() const
{
    return _impl->_dataSource;
}

uint32_t TextureDataCache::getTextureType() const
{
    return _impl->_textureType;
}

}
