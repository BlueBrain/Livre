
/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet.Bilgili@epfl.ch
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

#include <livre/Lib/Visitor/CollectionTraversal.h>

#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Visitor/RenderNodeVisitor.h>
#include <livre/core/Visitor/VisitState.h>

namespace livre
{

bool CollectionTraversal::traverse( DashNodeVector& dashNodeVector,
                                    RenderNodeVisitor& visitor,
                                    const bool reverse )
{
    DashNodeVector::const_iterator begin;
    DashNodeVector::const_iterator end;

    if( reverse )
    {
        begin = dashNodeVector.end();
        end = dashNodeVector.begin();
    }
    else
    {
        begin = dashNodeVector.begin();
        end = dashNodeVector.end();
    }

    VisitState state;
    visitor.onTraverseBegin( state );

    if( !state.getBreakTraversal() )
    {
        for( DashNodeVector::const_iterator i = begin; i != end;
             reverse ? --i : ++i )
        {
            const dash::NodePtr& node = *i;
            DashRenderNode renderNode( node );
            visitor.visit( renderNode, state );
            if( state.getBreakTraversal( ) )
                break;
        }
    }

    visitor.onTraverseEnd( state );
    return state.getBreakTraversal();
}

}

