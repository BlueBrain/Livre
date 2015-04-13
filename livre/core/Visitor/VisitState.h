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

#ifndef _VisitState_h_
#define _VisitState_h_

#include <string>
#include <livre/core/Data/LODNode.h>

namespace livre
{
/**
 * The VisitState class is used to keep visit states through traversal. Not thread-safe.
 */
class VisitState
{

public:

     VisitState( );

    /**
     * @return True, if children should be visited.
     */
    bool getVisitChild( ) const;

    /**
     * @return True, if traversal should be broken.
     */
    bool getBreakTraversal( ) const;

    /**
     * @return True, if neigbours should be visited.
     */
    bool getVisitNeighbours( ) const;

    /**
     * @param visitChild If true, informs the callee, not to visit children.
     */
    void setVisitChild( const bool visitChild );

    /**
     * @param breakVisitTraversal If true, traversal is broken.
     */
    void setBreakTraversal( const bool breakVisitTraversal );

    /**
     * @param visitNeighbours If false, neighbours won't be visited by the callee.
     */
    void setVisitNeighbours( const bool visitNeighbours );

protected:

    bool visitChild_;
    bool breakVisitTraversal_;
    bool visitNeighbours_;
};

}

#endif // _VisitState_h_
