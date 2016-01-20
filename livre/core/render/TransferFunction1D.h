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

#ifndef _TransferFunction1D_h_
#define _TransferFunction1D_h_

#include <co/dataIStream.h>
#include <co/dataOStream.h>

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/mathTypes.h>

#define TF_NCHANNELS 4u

namespace livre
{

/**
 * The TransferFunction1D class holds the color and transparency for an RGBA 1 dimensional Transfer Function ( TF ).
 */
class TransferFunction1D
{
public:
    /**
     * Create the transfer function with default parameters.
     */
    TransferFunction1D() { reset(); }

    /**
     * Create a transfer function.
     * @param size The number of samples in the transfer function.
     */
    explicit TransferFunction1D( const uint32_t size ) { createCustomTF_( size ); }

    /**
     * Load transfer function from an ASCII "1dt" file. The content of the file
     * consists of a first line with the number of sample points in the transfer
     * function and their format, and then all the values in 'R G B A' format.
     * Currently both float [0.0f, 1.0f] and 8-bit unsigned integers [0, 255]
     * values are supported. If the format is unspecified, float is used.
     * If the file extension or format is not supported or the file could not be
     * opened, a default transfer function is generated.
     * @param file Path to the transfer function file.
     */
    explicit TransferFunction1D( const std::string& file ) { createTfFromFile_( file ); }

    /**
     * Copy a transfer function.
     * @param tf The transfer function to be copied.
     */
    explicit TransferFunction1D( const TransferFunction1D& tf )
        : rgba_( tf.rgba_ )
    {}

    /**
     * Create a transfer function.
     * @param rgba A std::vector with samples of the transfer function.
     */
    explicit TransferFunction1D( const std::vector< uint8_t >& rgba )
        : rgba_( rgba )
    {}

    TransferFunction1D& operator=( const TransferFunction1D& rhs )
    {
        if( this == &rhs )
            return *this;

        rgba_ = rhs.rgba_;
        return *this;
    }

    /**
     * Resets the transfer function with default parameters.
     */
    LIVRECORE_API void reset();

    /**
     * @return The RGBA data vector. The data array is rgba_[] = { R, G, B, A, R, G, B, A, R ... }.
     */
    std::vector< uint8_t >& getData() { return rgba_; }

    /**
     * @return The RGBA data vector. The data array is rgba_[] = { R, G, B, A, R, G, B, A, R ... }.
     */
    const std::vector< uint8_t >& getData() const { return rgba_; }

    static uint32_t getNumChannels() { return TF_NCHANNELS; }

private:
    std::vector< uint8_t > rgba_;

    friend co::DataOStream& operator<<( co::DataOStream& os,
                                        const TransferFunction1D& tf );
    friend co::DataIStream& operator>>( co::DataIStream& is,
                                        TransferFunction1D& tf );

    LIVRECORE_API void createCustomTF_( const uint32_t size );

    LIVRECORE_API void createTfFromFile_( const std::string& file );
};

inline
co::DataOStream& operator<<( co::DataOStream& os, const TransferFunction1D& tf )
{
    return os << tf.getData();
}

inline
co::DataIStream& operator>>( co::DataIStream& is, TransferFunction1D& tf )
{
    return is >> tf.getData();
}
}
#endif // _TransferFunction1D_h_
