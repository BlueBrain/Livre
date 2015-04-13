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

#include <livre/Lib/Visitor/DFSTraversal.h>
#include <livre/core/Visitor/VisitState.h>

namespace livre
{

DFSTraversal::DFSTraversal( )
{
}

bool DFSTraversal::traverse( dash::NodePtr root,
                             NodeVisitor< dash::NodePtr > &visitor )
{
    if( !root )
        return false;

    state_ = VisitState();

    visitor.onTraverseBegin( state_ );
    if( state_.getBreakTraversal( ) )
        return true;

    traverse_( root, visitor );
    visitor.onTraverseEnd( state_ );

    bool retVal = state_.getBreakTraversal();
    state_.setBreakTraversal( false );

    return retVal;
}

void DFSTraversal::traverse_( dash::NodePtr dashNode, NodeVisitor< dash::NodePtr >& visitor )
{
    if( state_.getBreakTraversal() )
        return;

    visitor.onVisitBegin( dashNode, state_ );
    visitor.visit( dashNode, state_ );
    visitor.onVisitEnd( dashNode, state_ );

    if( state_.getBreakTraversal() || !state_.getVisitChild() )
    {
        state_.setVisitChild( true );
        return;
    }

    if( !dashNode->getNChildren() )
        return;

    visitor.onVisitChildrenBegin( dashNode, state_ );
    const int32_t nbOfChildren = dashNode->getNChildren();

    for( int32_t i = 0; i < nbOfChildren; ++i )
    {
        traverse_( dashNode->getChild( i ), visitor );
        if( !state_.getVisitNeighbours( ) )
        {
            break;
        }
    }

    visitor.onVisitChildrenEnd( dashNode, state_ );
    state_.setVisitNeighbours( true );
}

}
