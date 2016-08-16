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

#include <livre/lib/cache/DataCache.h>
#include <livre/lib/cache/DataObject.h>

#include <livre/core/data/LODNode.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/render/Renderer.h>
#include <livre/core/cache/CacheStatistics.h>

#include <eq/gl.h>

namespace livre
{

struct DataCache::Impl
{
public:
    Impl( DataSource& dataSource )
        : _dataSource( dataSource )
    {}

    CacheObject* generate( const CacheId& cacheId, DataCache& cache )
    {
        return new DataObject( cacheId, cache );
    }

    DataSource& _dataSource;
};

DataCache::DataCache( const size_t maxMemBytes, DataSource& dataSource )
    : Cache( "Data cache CPU", maxMemBytes )
    , _impl( new Impl( dataSource ))
{}

CacheObject* DataCache::_generate( const CacheId& cacheId )
{
    return _impl->generate( cacheId, *this );
}

DataCache::~DataCache()
{
    _unloadAll();
}

DataSource& DataCache::getDataSource()
{
    return _impl->_dataSource;
}

const DataSource& DataCache::getDataSource() const
{
    return _impl->_dataSource;
}

}
