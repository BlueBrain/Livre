/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
 *                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#include "TransferFunction1D.h"

#include <lunchbox/file.h>

#include <fstream>
#include <iterator>

namespace livre
{

TransferFunction1D::TransferFunction1D()
{
    const Vector3f color1( 0.0f, 1.0f, 1.0f );
    const Vector3f color2( 1.0f, 0.0f, 1.0f );
    const float alpha1 = 0.39f;
    const float alpha2 = 0.03f;
    const float density1 = 127.f;
    const float density2 = 255.f;
    const Vector3f colorDiff = color2 - color1;
    const float alphaDiff = alpha2 - alpha1;

    uint8_t* data = getLut();

    for( uint32_t i = 0; i < getLutSize(); i += NCHANNELS )
    {
        Vector4f rgba;
        if( i > 0 && i <= density1 )
        {
            rgba = color1;
            rgba[3] = alpha1 * ( float(i) / density1 );
        }
        else if( i > 0 )
        {
            rgba = ( float( i ) - density1 + 1.f ) /
                   ( density2 - density1 + 1.f ) * colorDiff + color1;
            rgba[3] = ( float( i ) - density1 + 1.f ) /
                      ( density2 - density1 + 1.f ) * alphaDiff + alpha1;
        }

        float maxUint8 = std::numeric_limits< uint8_t >::max();
        data[ i + 0 ] = rgba[0] * maxUint8;
        data[ i + 1 ] = rgba[1] * maxUint8;
        data[ i + 2 ] = rgba[2] * maxUint8;
        data[ i + 3 ] = rgba[3] * maxUint8;
    }
}

void TransferFunction1D::_createTfFromFile( const std::string& file )
{
    if( file.substr( file.find_last_of(".") + 1 ) == "lbb" )
    {
        lunchbox::loadBinary( *this, file );
        return;
    }
    if( file.substr( file.find_last_of(".") + 1 ) == "lba" )
    {
        lunchbox::loadAscii( *this, file );
        return;
    }

    std::ifstream ifs( file );
    if( !ifs.is_open( ))
    {
        LBWARN << "Transfer function file " << file << " could not be opened."
               << std::endl;
        return;
    }

    std::string line, val;
    std::getline( ifs, line );
    const std::string& formatStr = line.substr( line.find(' ') + 1 );
    const size_t numValues = atoi( line.c_str( )) * NCHANNELS;
    if( !numValues || numValues != getLutSize( ))
    {
        LBWARN << "Wrong format in transfer function file: " << file
               << ", got " << numValues << " entries, expect " << getLutSize()
               << std::endl;
        return;
    }

    size_t i = 0;
    uint8_t* data = getLut();
    const bool hasBytes = (formatStr == "uint8");
    while( ifs >> val && i < numValues )
    {
        data[i++] = hasBytes ? std::stoi( val ) :
                        std::stof( val ) * std::numeric_limits< uint8_t >::max();
    }
}
}
