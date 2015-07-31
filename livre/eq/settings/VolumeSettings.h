
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *               2013, Ahmet Bilgili  <ahmet.bilgili@epfl.ch>
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

/**
 * @brief The VolumeSettings class, keeps basic information about the volume data.
 */
class VolumeSettings : public co::Serializable
{
public:

    /**
     * @brief VolumeSettings constructor.
     */
    VolumeSettings( );

    /**
     * @brief reset Resets the information.
     */
    void reset( );

    /**
     * @brief setModelFileName Sets the models source uri.
     * @param uri The uri of the source data.
     */
    void setURI( const std::string& uri );

    /**
     * @return Returns the uri of the source data.
     */
    const std::string& getURI( ) const;

private:

    virtual void serialize(   co::DataOStream& os, const uint64_t dirtyBits );
    virtual void deserialize( co::DataIStream& is, const uint64_t dirtyBits );

    std::string uri_;
};

}

#endif // _VolumeInfo_h_
