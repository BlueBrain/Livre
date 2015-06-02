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

#include <livre/core/Visitor/VisitState.h>
#include <livre/core/Visitor/NodeVisitor.h>
#include <livre/core/Dash/DashTree.h>
#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Data/VolumeDataSource.h>
#include <livre/Lib/Visitor/DFSTraversal.h>
#include <livre/core/Visitor/VisitState.h>

namespace livre
{

namespace detail
{

class DFSTraversal
{
public:
    bool traverse( const NodeId& nodeId,
                   const uint32_t depth,
                   livre::NodeVisitor& visitor )
    {
        if( depth == 0 || _state.getBreakTraversal() )
            return false;

        _state = VisitState();

        visitor.onTraverseBegin( _state );
        if( _state.getBreakTraversal( ) )
            return true;

        visitor.onVisitBegin( nodeId, _state );
        visitor.visit( nodeId, _state );
        visitor.onVisitEnd( nodeId, _state );

        if( _state.getBreakTraversal() || !_state.getVisitChild() )
        {
            _state.setVisitChild( true );
            return false;
        }

        const NodeIds& nodeIds = nodeId.getChildren();
        visitor.onVisitChildrenBegin( nodeId, _state );
        BOOST_FOREACH( const NodeId& childNodeId, nodeIds )
        {
            traverse( childNodeId, depth - 1, visitor );
            if( !_state.getVisitNeighbours() )
                break;
        }

        visitor.onVisitChildrenEnd( nodeId, _state );
        _state.setVisitNeighbours( true );

        visitor.onTraverseEnd( _state );

        bool retVal = _state.getBreakTraversal();
        _state.setBreakTraversal( false );

        return retVal;
    }

    VisitState _state; //!< Status of the travel
};

}


DFSTraversal::DFSTraversal( )
    : _impl( new detail::DFSTraversal() )
{
}

DFSTraversal::~DFSTraversal()
{
    delete _impl;
}

bool DFSTraversal::traverse( const RootNode& rootNode,
                             const NodeId& node,
                             NodeVisitor& visitor )
{
    return _impl->traverse( node,
                            rootNode.getDepth(),
                            visitor );
}

void DFSTraversal::traverse( const RootNode& rootNode,
                             NodeVisitor& visitor)
{
    const Vector3ui& blockSize = rootNode.getBlockSize();
    for( uint32_t x = 0; x < blockSize[0]; ++x )
        for( uint32_t y = 0; y < blockSize[1]; ++y )
            for( uint32_t z = 0; z < blockSize[2]; ++z )
            {
                _impl->traverse( NodeId( 0, Vector3ui( x, y, z )),
                                 rootNode.getDepth(),
                                 visitor );
            }
}


}
