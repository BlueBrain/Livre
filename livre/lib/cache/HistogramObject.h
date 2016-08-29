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

#include <livre/lib/api.h>
#include <livre/lib/types.h>

#include <livre/core/mathTypes.h>
#include <livre/core/cache/CacheObject.h> // base class

namespace livre
{

/**
 * The HistogramObject class holds the histogram per texture raw data.
 */
class HistogramObject : public CacheObject
{
public:

    /**
     * Constructor
     * @param cacheId is the unique identifier
     * @param dataCache the histogram source data is retrieved from data cache
     * @param dataSource the data source
     * @param dataSourceRange range of the data source. The range is expanded with the loaded
     * data
     * @throws CacheLoadException when the data cache does not have the data for cache id
     */
    LIVRE_API HistogramObject( const CacheId& cacheId,
                               const Cache& dataCache,
                               const DataSource& dataSource,
                               const Vector2f& dataSourceRange );

    LIVRE_API ~HistogramObject();

    /** @copydoc livre::CacheObject::getSize */
    LIVRE_API size_t getSize() const final;

    /** @return the histogram */
    LIVRE_API const Histogram& getHistogram() const;

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;

};

}

#endif // _HistogramObject_h_
