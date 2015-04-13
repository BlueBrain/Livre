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

namespace livre
{

VisitState::VisitState( )
    : visitChild_( true ),
      breakVisitTraversal_( false ),
      visitNeighbours_( true )
{}

bool VisitState::getVisitChild( ) const
{
    return visitChild_;
}

bool VisitState::getBreakTraversal( ) const
{
    return breakVisitTraversal_;
}

bool VisitState::getVisitNeighbours( ) const
{
    return visitNeighbours_;
}

void VisitState::setVisitChild( const bool visitChild )
{
    visitChild_ = visitChild;
}

void VisitState::setBreakTraversal( const bool breakVisitTraversal )
{
    breakVisitTraversal_ = breakVisitTraversal;
}

void VisitState::setVisitNeighbours( const bool visitNeighbours )
{
    visitNeighbours_ = visitNeighbours;
}


}
