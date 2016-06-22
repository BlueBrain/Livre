/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#ifndef _HistogramObject_h_
#define _HistogramObject_h_

#include <livre/lib/types.h>

#include <livre/core/cache/CacheObject.h> // base class

namespace livre
{

/**
 * The HistogramObject class holds the histogram per texture data object. As textureData
 * object holds the converted data for rendering, histogram represents the rendered data,
 * not the original data.
 */
class HistogramObject : public CacheObject
{
public:
    ~HistogramObject();

    Histogram getHistogram() const;

private:

    friend class HistogramCache;

    HistogramObject( const CacheId& cacheId,
                     const HistogramCache& histogramCache );

    size_t _getSize() const;
    bool _load() final;
    void _unload() final;
    bool _isLoaded() const final;

    struct Impl;
    std::unique_ptr<Impl> _impl;

};

}

#endif // _HistogramObject_h_
