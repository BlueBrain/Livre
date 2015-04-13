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

#include <livre/Lib/Visitor/BFSTraversal.h>
#include <livre/core/Visitor/VisitState.h>

namespace livre
{

BFSTraversal::BFSTraversal( )
{
}

bool BFSTraversal::traverse( dash::NodePtr root,
                             NodeVisitor< dash::NodePtr > &visitor )
{
    state_ = VisitState();

    visitor.onTraverseBegin( state_ );
    if( state_.getBreakTraversal() )
        return false;

    std::deque< dash::NodePtr > bfsQueue;

    bfsQueue.push_back( root );

    dash::NodePtr currentParent;

    bool isBeginning = false;

    while( !bfsQueue.empty() )
    {
        dash::NodePtr node = bfsQueue.front();
        bfsQueue.pop_front();

        visitor.onVisitBegin( node, state_ );
        visitor.visit( node, state_ );
        visitor.onVisitEnd( node, state_ );

        if( node->hasParents() )
            currentParent = node->getParent( 0 );

        if( isBeginning )
        {
            visitor.onVisitChildrenBegin( node, state_ );
            isBeginning = false;
        }

        if( state_.getBreakTraversal( ) )
            return false;

        if( !state_.getVisitNeighbours( ) && currentParent )
        {
            dash::NodePtr parent = bfsQueue.back()->getParent( 0 );
            while( parent == currentParent )
            {
                bfsQueue.pop_back();
                if( bfsQueue.empty() )
                    break;
                parent = bfsQueue.back()->getParent( 0 );
            }
        }

        if( state_.getVisitChild( ) )
        {
            const int32_t nbOfChildren = node->getNChildren();
            for( int32_t i = 0; i < nbOfChildren; ++i )
            {
                bfsQueue.push_back( node->getChild( i ) );
            }
        }

        if( !bfsQueue.empty() && bfsQueue.front()->getParent( 0 ) != currentParent && node != root )
        {
           visitor.onVisitChildrenEnd( node, state_ );
           isBeginning = true;
        }
    }

    visitor.onTraverseEnd( state_ );
    bool retValue = state_.getBreakTraversal();
    state_.setBreakTraversal( false );
    return retValue;
}


}
