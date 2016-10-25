/* Copyright (c) 2016, EPFL/Blue Brain Project
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

#ifndef _ClipPlanes_h_
#define _ClipPlanes_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/mathTypes.h>
#include <co/distributable.h> // base class
#include <lexis/render/clipPlanes.h> // CRTP base class


namespace livre
{

class ClipPlanes : public co::Distributable< ::lexis::render::ClipPlanes >
{
public:
    /**
     * Constructor. Adds 6 othogonal planes in normalized space
     * (+x,-x,+y,-y,+z,-z). Convex region is defined as an AABB
     * ( -0.5,-0.5,-0.5 ) to ( 0.5, 0.5, 0.5 )
     */
    LIVRECORE_API ClipPlanes();

    /**
     * Copy the clip planes.
     * @param rhs The clip planes to be copied.
     */
    LIVRECORE_API ClipPlanes( const ClipPlanes& rhs );
    LIVRECORE_API ClipPlanes& operator=( const ClipPlanes& rhs );

    /**
     * @return true if there are no clipping planes
     */
    LIVRECORE_API bool isEmpty() const;

    /**
     * @return true if there are no clipping planes
     */
    LIVRECORE_API void clear();

    /**
     * @return true if there are no clipping planes
     */
    LIVRECORE_API void reset();

    /**
     * @return false if the clipping planes intersect or include the volume.
     */
    LIVRECORE_API bool isClipped( const Boxf& box ) const;
};

}
#endif // _ClipPlanes_h_
