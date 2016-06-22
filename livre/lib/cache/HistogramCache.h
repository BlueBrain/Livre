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

#ifndef _HistogramCache_h_
#define _HistogramCache_h_

#include <livre/lib/api.h>
#include <livre/lib/types.h>

#include <livre/core/cache/Cache.h>

namespace livre
{

/**
 * The HistogramCache class stores the histogram per node in the LOD octree
 */
class HistogramCache : public Cache
{
public:
    /**
     * @param maxMemBytes Maximum data memory
     * @param dataCache the data cache to read histogram from.
     */
    LIVRE_API HistogramCache( size_t maxMemBytes,
                              const TextureDataCache& dataCache );

    ~HistogramCache();

    /**
     * @return the data cache
     */
    LIVRE_API const TextureDataCache& getDataCache() const;

private:

    CacheObject* _generate( const CacheId& cacheId );

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}
#endif //_HistogramCache_h_
