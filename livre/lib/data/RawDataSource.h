/* Copyright (c) 2016, EPFL/Blue Brain Project
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

#pragma once
#include <livre/core/data/DataSourcePlugin.h>

#include <livre/lib/types.h>

namespace livre
{
/**
 * Data source for *.[raw|img] data with given details or nrrd volume
 *
 * Volumes need to fit into the GPU memory. If the data does not
 * fit GPU memory, texture upload will fail with OpenGL error number 1281.
 *
 */
class RawDataSource : public DataSourcePlugin
{
public:
    RawDataSource(const DataSourcePluginData& initData);
    ~RawDataSource();

    /**
     * Read the data for a given node.
     * @param node LODNode to be read.
     * @return The block data for the node.
     */
    MemoryUnitPtr getData(const LODNode& node) final;
    static bool handles(const DataSourcePluginData& initData);
    static std::string getDescription();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};
}
