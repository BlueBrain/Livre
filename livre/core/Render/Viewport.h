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

#ifndef _ViewPort_h_
#define _ViewPort_h_

#include <livre/core/mathTypes.h>

namespace livre
{

/**
 * The Viewport class is used to define a rectangle on a surface.
 */
template< class T >
class Viewport
{

public:

    Viewport( ) :
        pos_( 0.0 ),
        size_( 0.0 )
    {}

    /**
     * @param pos Position of the viewport.
     * @param size Size of the viewport.
     */
    Viewport( const vmml::vector< 2, T >& pos, const vmml::vector< 2, T >& size )
    {
        set( pos, size );
    }

    /**
     * Sets the position and size of the viewport.
     * @param pos Position of the viewport.
     * @param size Size of the viewport.
     */
    void set( const vmml::vector< 2, T >& pos, const vmml::vector< 2, T >& size )
    {
        pos_ = pos;
        size_ = size;
    }

    /**
     * @return The position of the viewport.
     */
    const vmml::vector< 2, T >& getPosition( ) const
    {
        return pos_;
    }

    /**
     * @return The size of the viewport.
     */
    const vmml::vector< 2, T >& getSize( ) const
    {
        return size_;
    }

    /**
     * @return The size x position the viewport.
     */
    T getX( ) const
    {
        return pos_[ 0 ];
    }

    /**
     * @return The size y position the viewport.
     */
    T getY( ) const
    {
        return pos_[ 1 ];
    }

    /**
     * @return The width of the viewport.
     */
    T getWidth( ) const
    {
        return size_[ 0 ];
    }

    /**
     * @return The height of the viewport.
     */
    T getHeight( ) const
    {
        return size_[ 1 ];
    }

private:

    vmml::vector< 2, T > pos_;
    vmml::vector< 2, T > size_;
};

}
#endif
