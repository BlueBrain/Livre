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

#ifndef _RawDataSource_h_
#define _RawDataSource_h_

#include <livre/core/data/DataSourcePlugin.h>

namespace livre
{

/**
 * Provides a data source for *.[raw|img] data with given details or nrrd volume .
 * The limitation is, the volumes should fit into the GPU memory. If the data does not
 * fit GPU memory, texture upload will fail with OpenGL error number 1281.
 *
 * Parses URIs in the form: raw://filename.[raw|img]#1024,1024,1024,uint8 or
 *                          raw://filename.nrrd
 *
 */
class RawDataSource : public DataSourcePlugin
{
public:

    RawDataSource( const DataSourcePluginData& initData );
    ~RawDataSource();

    /**
     * Read the data for a given node.
     * @param node LODNode to be read.
     * @return The block data for the node.
     */
    MemoryUnitPtr getData( const LODNode& node ) final;

    static bool handles( const DataSourcePluginData& initData );
private:

    struct Impl;
    std::unique_ptr< Impl > _impl;
};

}

#endif // _RawDataSource_h_
