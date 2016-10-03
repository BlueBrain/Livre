
/* Copyright (c) 2011-2016, Maxim Makhinya <maxmah@gmail.com>
 *                          David Steiner  <steiner@ifi.uzh.ch>
 *                          Ahmet Bilgili  <ahmet.bilgili@epfl.ch>
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

#ifndef _VolumeSettings_h_
#define _VolumeSettings_h_

#include <livre/lib/types.h>
#include <co/serializable.h>

namespace livre
{

/** Basic information about the volume data. */
class VolumeSettings : public co::Serializable
{
public:

    /** VolumeSettings constructor. */
    VolumeSettings();

    /** reset Resets the information. */
    void reset();

    /**
     * Sets the models source uri.
     * @param uri The uri of the source data.
     */
    void setURI( const std::string& uri );

    /** @return Returns the uri of the source data. */
    const std::string& getURI( ) const;

    /**
     * Sets the accumulated data range for the volume.
     * @param range of the data source.
     */
    void setDataSourceRange( const Vector2f& range );

    /** @return Returns the accumulated data range for the volume. */
    const Vector2f& getDataSourceRange() const;

private:
    void serialize(   co::DataOStream& os, const uint64_t dirtyBits ) override;
    void deserialize( co::DataIStream& is, const uint64_t dirtyBits ) override;

    std::string _uri;
    Vector2f _dataSourceRange;
};

}

#endif // _VolumeInfo_h_
