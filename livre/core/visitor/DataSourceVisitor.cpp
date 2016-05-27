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

#include <livre/core/visitor/DataSourceVisitor.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/data/DataSource.h>

namespace livre
{

class DataSourceVisitor::Impl
{
public:
    explicit Impl( const DataSource& dataSource )
        : _dataSource( dataSource )
    {}

    LODNode getNode( const NodeId& nodeId ) const
    {
        return _dataSource.getNode( nodeId );
    }

    const DataSource& _dataSource;
};

DataSourceVisitor::DataSourceVisitor( const DataSource& dataSource )
    : _impl( new DataSourceVisitor::Impl( dataSource ))
{}

DataSourceVisitor::~DataSourceVisitor()
{}

const DataSource& DataSourceVisitor::getDataSource() const
{
    return _impl->_dataSource;
}

void DataSourceVisitor::visit( const NodeId& nodeId,
                               VisitState& state )
{
    const LODNode& node = _impl->getNode( nodeId );
    if( node.isValid( ))
        visit( node, state );
}

}

