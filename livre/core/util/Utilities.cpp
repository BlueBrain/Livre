/* Copyright (c) 2014, EPFL/Blue Brain Project
 *                     Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#ifndef _utility_h_
#define _utility_h_

#include <livre/core/util/Utilities.h>
#include <eq/gl.h>

namespace livre
{

bool hasOpenGL42Support()
{
#ifdef GL_MAJOR_VERSION
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    return ( major >= 5 ) || (( major == 4 ) && ( minor >= 2 ));
#else
    return false;
#endif
}

size_t getDataTypeSize( DataType dataType )
{
    switch( dataType )
    {
    case DT_FLOAT64:
        return 8;
    case DT_FLOAT32:
    case DT_UINT32:
    case DT_INT32:
        return 4;
    case DT_UINT16:
    case DT_INT16:
        return 2;
    case DT_UINT8:
    case DT_INT8:
        return 1;
    default:
        return DT_UNDEFINED;
    }
}

std::string getDataTypeSize( DataType dataType )
{
    switch( dataType )
    {
    case DT_FLOAT64:
        return "FLOAT64";
    case DT_FLOAT32:
        return "FLOAT32";
    case DT_UINT32:
        return "UINT32";
    case DT_INT32:
        return "INT32";
    case DT_UINT16:
        return "UINT16";
    case DT_INT16:
        return "INT16";
    case DT_UINT8:
        return "UINT8";
    case DT_INT8:
        return "INT8";
    default:
        return "UNDEFINED";
    }
}

DataType getDataTypeFromStr( const std::string& str )
{
    if( str == "FLOAT64" )
        return DT_FLOAT64;
    else if( str == "FLOAT32" )
        return DT_FLOAT32;
    else if( str == "UINT32" )
        return DT_UINT32;
    else if( str == "INT32" )
        return DT_INT32;
    else if( str == "UINT16" )
        return DT_UINT16;
    else if( str == "INT16" )
        return DT_INT16;
    else if( str == "INT8" )
       return DT_UINT8;
     else if( str == "INT8" )
        return DT_INT8;
    else
        return DT_UNDEFINED;
}

}

}

#endif // _utility_h_
