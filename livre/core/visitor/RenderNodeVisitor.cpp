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

#include <livre/core/visitor/RenderNodeVisitor.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/data/VolumeDataSource.h>

namespace livre
{

class RenderNodeVisitor::Impl
{
public:
    explicit Impl( const VolumeDataSource& dataSource )
        : _dataSource( dataSource )
    {}

    LODNode getLODNode( const NodeId& nodeId )
    {
        return _dataSource.getNode( nodeId );
    }

    LODNode getLODNode( const NodeId& nodeId ) const
    {
        return _dataSource.getNode( nodeId );
    }

    const VolumeDataSource& _dataSource;
};

RenderNodeVisitor::RenderNodeVisitor( const VolumeDataSource& dataSource )
    : _impl( new RenderNodeVisitor::Impl( dataSource ))
{

}

RenderNodeVisitor::~RenderNodeVisitor()
{
    
}

const VolumeDataSource& RenderNodeVisitor::getDataSource() const
{
    return _impl->_dataSource;
}

void RenderNodeVisitor::visit( const NodeId& nodeId,
                               VisitState& state )
{
    const LODNode& node = _impl->getLODNode( nodeId );
    if( node.isValid( ))
        visit( node, state );
}

}

