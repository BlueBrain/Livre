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

#include <livre/lib/cache/HistogramCache.h>
#include <livre/lib/cache/HistogramObject.h>

#include <livre/lib/cache/DataCache.h>
#include <livre/lib/cache/DataObject.h>

namespace livre
{

struct HistogramCache::Impl
{
public:
    Impl( const DataCache& dataCache )
        : _dataCache( dataCache )
    {}

   const DataCache& _dataCache;
};

HistogramCache::HistogramCache( const size_t maxMemBytes,
                                const DataCache& dataCache )
    : Cache( "Histogram Cache", maxMemBytes )
    , _impl( new Impl( dataCache ))
{}

CacheObject* HistogramCache::_generate( const CacheId& cacheId )
{
    return new HistogramObject( cacheId, _impl->_dataCache );
}

HistogramCache::~HistogramCache()
{
    _unloadAll();
}

const DataCache& HistogramCache::getDataCache() const
{
    return _impl->_dataCache;
}

}
