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

#include <livre/core/types.h>
#include <livre/core/mathTypes.h>

#define TF_NCHANNELS 4u

namespace livre
{

/**
 * The TransferFunction1D class holds the color and transparency for an RGBA 1 dimensional Transfer Function ( TF ).
 * The data type can be templated by parameter T.
 */
template< class T  >
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
    TransferFunction1D( const uint32_t size ) { createCustomTF_( size ); }

    /**
     * Copy a transfer function.
     * @param tf The transfer function to be copied.
     */
    TransferFunction1D( const TransferFunction1D< T >& tf )
        : rgba_( tf.rgba_ )
    {}

    /**
     * Create a transfer function.
     * @param rgba A std::vector with samples of the transfer function.
     */
    TransferFunction1D( const std::vector< T >& rgba )
        : rgba_( rgba )
    {}

    /**
     * Resets the transfer function with default parameters.
     */
    void reset();

    /**
     * @return The RGBA data vector. The data array is rgba_[] = { R, G, B, A, R, G, B, A, R .... }.
     */
    std::vector< T >& getData() { return rgba_; }

    /**
     * @return The RGBA data vector. The data array is rgba_[] = { R, G, B, A, R, G, B, A, R .... }.
     */
    const std::vector< T >& getData() const { return rgba_; }

    static uint32_t getNumChannels() { return TF_NCHANNELS; }

private:
    std::vector< T > rgba_;

    template <typename U>
    friend co::DataOStream& operator<<( co::DataOStream& os, const TransferFunction1D< U >& tf );

    template <typename U>
    friend co::DataIStream& operator>>( co::DataIStream& is, TransferFunction1D< U >& tf );

    void createCustomTF_( const uint32_t size );
};

/**
  * Typedefs for templates
  */
typedef TransferFunction1D< uint8_t > TransferFunction1Dc;
typedef TransferFunction1D< float > TransferFunction1Df;
typedef boost::shared_ptr< TransferFunction1Df > TransferFunction1DfPtr;

template < class U >
co::DataOStream& operator<<( co::DataOStream& os,
                             const TransferFunction1D< U >& tf )
{
    return os << tf.getData();
}

template < class U >
co::DataIStream& operator>>( co::DataIStream& is, TransferFunction1D< U >& tf )
{
    return is >> tf.getData();
}
}
#endif // _TransferFunction1D_h_
