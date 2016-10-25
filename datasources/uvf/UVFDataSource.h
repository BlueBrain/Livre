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

#ifndef _UVFDataSource_h_
#define _UVFDataSource_h_

#include <livre/core/data/DataSourcePlugin.h>

namespace livre
{

/** Reads Tuvok Volumes and generates hierarchies. */
class UVFDataSource : public DataSourcePlugin
{
public:
    UVFDataSource( const DataSourcePluginData& initData );
    virtual ~UVFDataSource();

    static bool handles( const DataSourcePluginData& initData );

private:

    MemoryUnitPtr getData( const LODNode& node ) final;
    LODNode internalNodeToLODNode( const NodeId& internalNode ) const final;

    struct Impl;
    std::unique_ptr< Impl > _impl;
};

}

#endif // _UVFDataSource_h_
