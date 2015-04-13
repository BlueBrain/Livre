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

#include <livre/core/Data/VolumeDataSourcePlugin.h>

#include <livre/Lib/types.h>

namespace livre
{

/**
 * Generates random, in-memory volume data.
 *
 * Parses URIs in the form:   mem:///#1024,1024,1024,32
 */
class MemoryDataSource : public VolumeDataSourcePlugin
{
public:

    MemoryDataSource( const VolumeDataSourcePluginData& initData );
    virtual ~MemoryDataSource();

    /**
     * Read the data for a given node.
     * @param node LODNode to be read.
     * @return The block data for the node.
     */
    MemoryUnitPtr getData( const LODNode& node ) override;
    static bool handles( const VolumeDataSourcePluginData& initData );

};

}

#endif // _MemoryDataSource_h_
