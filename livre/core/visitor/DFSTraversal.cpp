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

#include <livre/core/visitor/VisitState.h>
#include <livre/core/visitor/NodeVisitor.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/visitor/DFSTraversal.h>
#include <livre/core/visitor/VisitState.h>

namespace livre
{

struct DFSTraversal::Impl
{
public:

    bool traverse( const NodeId& nodeId,
                   const uint32_t depth,
                   livre::NodeVisitor& visitor )
    {
        if( depth == 0 || _state.getBreakTraversal() )
            return false;

        _state = VisitState();

        if( _state.getBreakTraversal( ) )
            return true;

        visitor.visit( nodeId, _state );

        if( _state.getBreakTraversal() || !_state.getVisitChild() )
        {
            _state.setVisitChild( true );
            return false;
        }

        const NodeIds& nodeIds = nodeId.getChildren();
        for( const NodeId& childNodeId: nodeIds )
        {
            traverse( childNodeId, depth - 1, visitor );
            if( !_state.getVisitNeighbours() )
                break;
        }

        _state.setVisitNeighbours( true );

        bool retVal = _state.getBreakTraversal();
        _state.setBreakTraversal( false );

        return retVal;
    }

    VisitState _state; //!< Status of the travel
};

DFSTraversal::DFSTraversal( )
    : _impl( new DFSTraversal::Impl() )
{
}

DFSTraversal::~DFSTraversal()
{
    
}

bool DFSTraversal::traverse( const RootNode& rootNode, const NodeId& node,
                             NodeVisitor& visitor )
{
    visitor.visitPre();
    const bool ret = _impl->traverse( node, rootNode.getDepth(), visitor );
    visitor.visitPost();
    return ret;
}

void DFSTraversal::traverse( const RootNode& rootNode, NodeVisitor& visitor,
                             const uint32_t frame )
{
    visitor.visitPre();
    const Vector3ui& blockSize = rootNode.getBlockSize();
    for( uint32_t x = 0; x < blockSize.x(); ++x )
        for( uint32_t y = 0; y < blockSize.y(); ++y )
            for( uint32_t z = 0; z < blockSize.z(); ++z )
            {
                _impl->traverse( NodeId( 0, Vector3ui( x, y, z ), frame ),
                                 rootNode.getDepth(), visitor );
            }
    visitor.visitPost();
}


}
