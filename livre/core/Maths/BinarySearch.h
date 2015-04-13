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

#ifndef _BinarySearch_h_
#define _BinarySearch_h_

#include <livre/core/types.h>

namespace livre
{

/**
 * Searches a value between limits, for a given function.
 * @param function is an object that can run operator( T )
 * @param begin is the lower limit.
 * @param end is the upper limit.
 * @param y the value to search.
 * @param epsilon is the error to stop.
 * @param error is true if it cannot find the valu between given limits.
 * @return The value between limits.
 */
template < class FunctionClass, class T, class U >
T binarySearch( FunctionClass& function,
                T begin,
                T end,
                U y,
                T epsilon,
                bool &error )
{
    float l = begin;
    float h = end;

    float lVal = function( l );
    float hVal = function( h );

    if( y < lVal || y > hVal )
    {
        LBINFO << "y :" << y << " is not between " << lVal << " " << hVal << std::endl;
        error = true;
        return y > hVal ? h : l;
    }

    float x = ( l + h ) / 2.0;
    while( fabs( l - h ) > epsilon )
    {
        x = ( l + h ) / 2.0;
        const float functionVal = function( x );
        if( functionVal >= y )
            h = x;
        else if( functionVal < y )
            l = x;
    }
    error = false;
    return h;
}

}

#endif // _BinarySearch_h_
