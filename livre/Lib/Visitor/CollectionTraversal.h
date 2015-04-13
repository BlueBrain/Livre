
/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
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

#ifndef _CollectionTraversal_h_
#define _CollectionTraversal_h_

#include <livre/core/types.h>

#include <livre/core/Visitor/NodeVisitor.h>
#include <livre/core/Visitor/VisitState.h>

#include <boost/type_traits.hpp>
#include <algorithm>

namespace livre
{
/**
 * @brief The CollectionTraversal class is used to traverse the stl containers ( or other classes supporting
 * iterators ).
 */
template < typename T, bool reverse = false > class CollectionTraversal
{
    typedef typename T::value_type value_type;

public:

    /**
     * @brief CollectionTraversal constrcutor.
     * @param container The container object.
     * @param reverse If true the container is iterated in reverse.
     */
    CollectionTraversal() {}

    /**
     * @brief traverse Starts the traversing of collection.
     * @param root the (backward-)iterable collection.
     * @param visitor Visitors visit is revoked for each visit of each collection object.
     * @return Returns true if traversal is completed, without being broken.
     */
    bool traverse( T root, NodeVisitor< value_type >& visitor )
    {
        typename T::const_iterator begin;
        typename T::const_iterator end;

        if( reverse )
        {
            begin = root.end();
            end = root.begin();
        }
        else
        {
            begin = root.begin();
            end = root.end();
        }

        VisitState state;
        visitor.onTraverseBegin( state );

        if( !state.getBreakTraversal() )
        {
            for( typename T::const_iterator i = begin; i != end;
                 reverse ? --i : ++i )
            {
                visitor.visit( *i, state );
                if( state.getBreakTraversal( ) )
                    break;
            }
        }

        visitor.onTraverseEnd( state );

        return state.getBreakTraversal();
    }
};

}

#endif // _CollectionTraversal_h_
