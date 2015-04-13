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

#ifndef _DashTree_h_
#define _DashTree_h_

#include <dash/dash.h>

#include <livre/core/types.h>

namespace livre
{

/**
 * The DashTree class to access the root of the dash octree.
 */
class DashTree
{
public:

    DashTree( );

    DashTree( const DashTree& dashTree );

    /**
     * @return The root dash node of the dash tree.
     */
    const dash::NodePtr getRootNode( ) const { return rootNode_; }

    /**
     * @return The root dash node of the dash tree.
     */
    dash::NodePtr getRootNode( ) { return rootNode_; }

private:

    dash::NodePtr rootNode_;

};

}

#endif // _DashTree_h_
