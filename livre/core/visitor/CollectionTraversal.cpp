
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

#include <livre/core/visitor/CollectionTraversal.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/visitor/RenderNodeVisitor.h>
#include <livre/core/visitor/VisitState.h>

namespace livre
{

bool CollectionTraversal::traverse( const NodeIds& nodeIds,
                                    RenderNodeVisitor& visitor,
                                    const bool reverse )
{
    NodeIds::const_iterator begin;
    NodeIds::const_iterator end;

    if( reverse )
    {
        begin = nodeIds.end();
        end = nodeIds.begin();
    }
    else
    {
        begin = nodeIds.begin();
        end = nodeIds.end();
    }

    VisitState state;
    visitor.visitPre();

    for( NodeIds::const_iterator i = begin; i != end;
         reverse ? --i : ++i )
    {
        const LODNode& lodNode = visitor.getDataSource().getNode( *i );
        visitor.visit( lodNode, state );
        if( state.getBreakTraversal( ) )
            break;
    }

    visitor.visitPost();
    return state.getBreakTraversal();
}

}
