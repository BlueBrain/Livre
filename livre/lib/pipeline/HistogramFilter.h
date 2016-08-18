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

#ifndef _HistogramFilter_h_
#define _HistogramFilter_h_

#include <livre/lib/types.h>

#include <livre/core/pipeline/Filter.h>

namespace livre
{

/**
 * Histogram filter computes the accumulated histogram for given node ids that
 * are in or intersecting the frustum.
 *
 */
class HistogramFilter : public Filter
{

public:

    /**
     * Constructor
     * @param histogramCache the cache for histogram
     * @param dataCache data cache
     * @param dataSource data source
     */
    HistogramFilter( Cache& histogramCache,
                     const Cache& dataCache,
                     const DataSource& dataSource );
    ~HistogramFilter();

    /**
     * @copydoc Filter::execute
     */
    void execute( const FutureMap& input, PromiseMap& output ) const final;

    /**
     * @copydoc Filter::getInputDataInfos
     */
    DataInfos getInputDataInfos() const final;

    /**
     * @copydoc Filter::getOutputDataInfos
     */
    DataInfos getOutputDataInfos() const final;

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif

