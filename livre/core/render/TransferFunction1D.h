/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#include <livre/core/api.h>
#include <livre/core/types.h>

#include <lexis/render/lookupTable1D.h>

namespace livre
{

/** Color and transparency for an RGBA 1 dimensional Transfer Function. */
class TransferFunction1D : public ::lexis::render::LookupTable1D
{
    static const size_t NCHANNELS = 4;

public:
    /** Create the transfer function with default parameters. */
    LIVRECORE_API TransferFunction1D();

    /**
     * Load transfer function.
     *
     * Supported are ASCII "1dt", as well as ascii and binary files of
     * servus::Serializable (.lba, .lbb).
     *
     * The content of the ASCII file consists of a first line with the number of
     * sample points in the transfer function and their format, and then all the
     * values in 'R G B A' format.  Currently both float [0.0f, 1.0f] and 8-bit
     * unsigned integers [0, 255] values are supported. If the format is
     * unspecified, float is used.  If the file extension or format is not
     * supported or the file could not be opened, a default transfer function is
     * generated.
     * @param file Path to the transfer function file.
     */
    explicit TransferFunction1D( const std::string& file )
        : TransferFunction1D() { _createTfFromFile( file ); }

    /**
     * Copy a transfer function.
     * @param tf The transfer function to be copied.
     */
    explicit TransferFunction1D( const TransferFunction1D& tf )
        : ::lexis::render::LookupTable1D( tf ) {}

    /**
     * Create a transfer function.
     * @param rgba A std::vector with samples of the transfer function.
     */
    explicit TransferFunction1D( const std::vector< uint8_t >& rgba )
        : ::lexis::render::LookupTable1D( rgba ) {}

    TransferFunction1D& operator=( const TransferFunction1D& rhs )
    {
        ::lexis::render::LookupTable1D::operator = ( rhs );
        return *this;
    }

    static uint32_t getNumChannels() { return NCHANNELS; }

private:
    LIVRECORE_API void _createTfFromFile( const std::string& file );
};
}
#endif // _TransferFunction1D_h_
