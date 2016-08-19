/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
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

#ifndef _MemoryDataSource_h_
#define _MemoryDataSource_h_

#include <livre/core/data/DataSourcePlugin.h>

#include <livre/lib/types.h>

namespace livre
{

/**
 * Generates in-memory volume data.
 *
 * Parses URIs in the form:
 *
 * mem:///?sparsity=1.0,datatype=[(u)int(8,16,32),float]#1024,1024,1024,32
 *
 * The "sparsity" parameter is the sparsity of the data between 0.0
 * and 1.0. 1.0 means no voxels will be empty. 0.0 means all voxels
 * will be empty. 0.001 means 99.9% of the voxels will be empty.
 *
 * The "datatype" parameter sets the volume data type.
 *
 * The rest of the parameters are total number of voxels in X,Y,Z and
 * the block size.
 */
class MemoryDataSource : public DataSourcePlugin
{
public:

    MemoryDataSource( const DataSourcePluginData& initData );
    virtual ~MemoryDataSource();

    /**
     * Read the data for a given node.
     * @param node LODNode to be read.
     * @return The block data for the node.
     */
    MemoryUnitPtr getData( const LODNode& node ) final;

    static bool handles( const DataSourcePluginData& initData );

private:
    float _sparsity;
};

}

#endif // _MemoryDataSource_h_
