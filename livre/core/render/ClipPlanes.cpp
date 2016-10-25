/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#include <livre/core/render/ClipPlanes.h>
#include <livre/core/data/VolumeInformation.h>
#include <livre/core/mathTypes.h>
#include <vmmlib/aabb.hpp>

namespace livre
{
namespace
{
float normals[ 6 ][ 3 ]  = {{ -1.0f, 0.0f, 0.0f }, /* +X plane */
                           {  1.0f, 0.0f, 0.0f  }, /* -X plane */
                           {  0.0f, -1.0f, 0.0f }, /* +Y plane */
                           {  0.0f, 1.0f, 0.0f  }, /* -Y plane */
                           {  0.0f, 0.0f, -1.0f }, /* +Z plane */
                           {  0.0f, 0.0f, 1.0f }}; /* -Z Plane */
}

ClipPlanes::ClipPlanes()
{
    reset();
}

ClipPlanes::ClipPlanes( const ClipPlanes& rhs )
    : co::Distributable< ::lexis::render::ClipPlanes >( rhs )
{
    if( this == &rhs )
        return;

    *this = rhs;
}

ClipPlanes& ClipPlanes::operator=( const ClipPlanes& rhs )
{
    if( this == &rhs )
        return *this;

    static_cast< ::lexis::render::ClipPlanes& >( *this ) = rhs;
    return *this;
}

bool ClipPlanes::isEmpty() const
{
    return getPlanes().empty();
}

void ClipPlanes::clear()
{
    getPlanes().clear();
}

void ClipPlanes::reset()
{
    clear();
    for( size_t i = 0; i < 6; ++i )
    {
        ::lexis::render::Plane plane;
        plane.setNormal( normals[ i ] );
        plane.setD( 0.5 );
        getPlanes().push_back( plane );
    }
}

bool ClipPlanes::isClipped( const Boxf& worldBox ) const
{
    const auto& vec = getPlanes();
    for( size_t i = 0; i < vec.size(); ++i )
    {
        const ::lexis::render::Plane& p = vec[ i ];
        const Vector3f& middle = worldBox.getCenter();
        const Vector3f& extent = worldBox.getSize() * 0.5f;

        const float* normal = p.getNormal();
        const Plane plane( normal[ 0 ], normal[ 1 ], normal[ 2 ], p.getD() );

        const float d = plane.dot( middle );
        const float n = extent.x() * std::abs( plane.x( )) +
                        extent.y() * std::abs( plane.y( )) +
                        extent.z() * std::abs( plane.z( ));

        if( !( d - n >= 0 || d + n > 0 ))
            return true;
    }

    return false;
}
}
