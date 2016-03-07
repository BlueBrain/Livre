/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <co/dataIStream.h>
#include <co/dataOStream.h>

#include <fstream>
#include <iterator>

#define DEFAULT_COLOR1_DENSITY 128u
#define DEFAULT_COLOR2_DENSITY 255u
#define DEFAULT_COLOR1_ALPHA 0.39f
#define DEFAULT_COLOR2_ALPHA 0.03f
#define DEFAULT_TF_SIZE 256u
#define DEFAULT_COLOR1_RGB Vector3f( 0.0f, 1.0f, 1.0f )
#define DEFAULT_COLOR2_RGB Vector3f( 1.0f, 0.0f, 1.0f )

namespace livre
{
namespace
{
enum SamplePointsType
{
    UINT8,
    FLOAT,
    DEFAULT = FLOAT
};

Floats create2ColorTF( const uint32_t surfaceColorPos,
                            const float surfaceColorAlpha,
                            const float solidColorAlpha, const size_t size,
                            const Vector3f& surfaceColorRGB,
                            const Vector3f& solidColorRGB )
{
    Vector3f finalColorRGB;
    float alpha;
    Floats transferFunction;

    const Vector3f colorDiff = solidColorRGB - surfaceColorRGB;
    const float alphaDiff = solidColorAlpha - surfaceColorAlpha;

    for( uint32_t i = 0; i < size; ++i )
    {
        if( i == 0 )
        {
            finalColorRGB = Vector3f(0.0f);
            alpha = 0;
        }
        else if( ( 0 < i ) && ( i<= surfaceColorPos ) )
        {
            finalColorRGB = surfaceColorRGB;
            alpha = surfaceColorAlpha * ( float(i) / (float)surfaceColorPos );
        }
        else
        {
            finalColorRGB = ( (float)( i - surfaceColorPos + 1u ) /
                              (float)( DEFAULT_COLOR2_DENSITY -
                                       surfaceColorPos + 1u ) ) * colorDiff +
                             surfaceColorRGB;
            alpha = ( (float)( i - surfaceColorPos + 1u ) /
                    (float)( DEFAULT_COLOR2_DENSITY - surfaceColorPos + 1u ) ) *
                    alphaDiff + surfaceColorAlpha;
        }

        transferFunction.push_back( finalColorRGB[0] );
        transferFunction.push_back( finalColorRGB[1] );
        transferFunction.push_back( finalColorRGB[2] );
        transferFunction.push_back( alpha );
    }
    return transferFunction;
}

Floats createDefault2ColorTF()
{
    return create2ColorTF( DEFAULT_COLOR1_DENSITY, DEFAULT_COLOR1_ALPHA,
                           DEFAULT_COLOR2_ALPHA,
                           DEFAULT_TF_SIZE, DEFAULT_COLOR1_RGB,
                           DEFAULT_COLOR2_RGB );
}

SamplePointsType readTransferFunction( const std::string& file,
                                       Floats& transferFunction )
{
    transferFunction = createDefault2ColorTF();
    if( file.empty( ))
    {
        LBWARN << "Using default transfer function" << std::endl;
        return DEFAULT;
    }
    if( file.substr( file.find_last_of(".") + 1 ) != "1dt" )
    {
        LBWARN << "Wrong transfer function file format: " << file
               << ", it must be '.1dt'. Using default transfer function"
               << std::endl;
        return DEFAULT;
    }

    std::ifstream ifs( file );
    if( !ifs.is_open( ))
    {
        LBWARN << "The specified transfer function file: " << file
               << ", could not be opened. Using default transfer function"
               << std::endl;
        return DEFAULT;
    }

    std::string line, val;
    std::getline( ifs, line );
    const std::string& formatStr = line.substr( line.find(' ') + 1 );
    const SamplePointsType format = formatStr == "uint8" ? UINT8 : DEFAULT;

    const size_t numValues = atoi( line.c_str( )) * TF_NCHANNELS;
    if( !numValues )
    {
        LBWARN << "Wrong format in transfer function file: " << file
               << ", the number of values must be specified. "
               << "Using default transfer function" << std::endl;
        return DEFAULT;
    }
    transferFunction.resize( numValues );
    size_t i = 0;
    while( ifs >> val && i < numValues )
        transferFunction[i++] = atof( val.c_str( ));

    return format;
}
}

void TransferFunction1D::reset()
{
    const Floats defaultTF = createDefault2ColorTF();
    const float maxVal = std::numeric_limits< uint8_t >::max();
    const size_t size = defaultTF.size();

    rgba_.resize( size );
    for( size_t i = 0; i < size; ++i )
        rgba_[i] = maxVal * defaultTF[i];
}

void TransferFunction1D::createCustomTF_( const uint32_t size )
{
    rgba_.resize( size * TF_NCHANNELS );

    rgba_[0] = 0u;
    rgba_[1] = 0u;
    rgba_[2] = 0u;
    rgba_[3] = 0u;
    for( size_t i = 1; i < size; ++i )
    {
        rgba_[ i * TF_NCHANNELS ] = 0u;
        rgba_[ i * TF_NCHANNELS + 1 ] = 0u;
        rgba_[ i * TF_NCHANNELS + 2 ] = 255u;
        rgba_[ i * TF_NCHANNELS + 3 ] = 2u;
    }
}

void TransferFunction1D::createTfFromFile_( const std::string& file )
{
    Floats transferFunction;
    const SamplePointsType& format = readTransferFunction( file, transferFunction );
    const size_t size = transferFunction.size();
    rgba_.resize( size );

    const uint8_t maxVal = std::numeric_limits< uint8_t >::max();
    const uint8_t conversionFactor = format == UINT8 ? 1u : maxVal;

    for( size_t i = 0; i < size; ++i )
        rgba_[i] = conversionFactor * transferFunction[i];
}
}
