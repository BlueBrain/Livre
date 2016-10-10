
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
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

#include <eq/eq.h>

#include <livre/eq/types.h>
#include <livre/eq/settings/EqVolumeSettings.h>

namespace livre
{

EqVolumeSettings::EqVolumeSettings()
{
    _uri =  SignalledVariable< std::string >( _uri.get(),
                                [this](const std::string&){ setDirty( DIRTY_ALL ); } );

    _dataSourceRange = SignalledVariable< Vector2f >( _dataSourceRange.get(),
                                    [this](const Vector2f&){ setDirty( DIRTY_ALL ); } );
}

void EqVolumeSettings::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );
    os  << _uri.get() << _dataSourceRange.get();
}

void EqVolumeSettings::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );
    is >> _uri.get() >> _dataSourceRange.get();
}

}
