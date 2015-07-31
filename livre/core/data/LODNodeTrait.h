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

#ifndef _LODNodeTrait_h_
#define _LODNodeTrait_h_

#include <livre/core/api.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * LODNodeTrait adds LODNode property to the derived classes.
 */
class LODNodeTrait
{
public:
    /**
     * @return \see LODNode.
     */
    LIVRECORE_API ConstLODNodePtr getLODNode() const;

protected:
    LIVRECORE_API LODNodeTrait();

    /**
     * @param lodNodePtr the livre::LODNode.
     */
    LIVRECORE_API LODNodeTrait( ConstLODNodePtr lodNodePtr );


    ConstLODNodePtr lodNodePtr_; //!< LODNode pointer

};


}

#endif // _LODNodeTrait_h_
