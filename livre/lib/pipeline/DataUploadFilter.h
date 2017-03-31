/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
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

#ifndef _DataUploadFilter_h_
#define _DataUploadFilter_h_

#include <livre/core/pipeline/Filter.h>
#include <livre/lib/types.h>

namespace livre
{
/**
 * DataUploadFilter class implements the parallel data loading for raw volume
 * data and textures.
 */
class DataUploadFilter : public Filter
{
public:
    /**
     * Constructor
     * @param dataCache data cache
     * @param textureCache texture cache
     * @param dataSource data source
     * @param texturePool the pool for 3D textures
     */
    DataUploadFilter(Cache& dataCache, Cache& textureCache,
                     DataSource& dataSource, TexturePool& texturePool);
    ~DataUploadFilter();

    /**
     * @copydoc Filter::execute
     */
    void execute(const FutureMap& input, PromiseMap& output) const final;

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
