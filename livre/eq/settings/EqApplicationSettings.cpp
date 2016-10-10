/*
 * Copyright (c) 2016, ahmetbilgili@gmail.com
 *
 * This file is part of Livre <https://github.com/bilgili/Livre>
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
#include <livre/eq/settings/EqApplicationSettings.h>

namespace livre
{
EqApplicationSettings::EqApplicationSettings()
{
    _resourceFolders =
            SignalledVariable< Strings >( _resourceFolders.get(),
                                          [this](const Strings&){ setDirty( DIRTY_ALL ); });
}

void EqApplicationSettings::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );
    os << _resourceFolders.get();
}

void EqApplicationSettings::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );
    is >> _resourceFolders.get();
}

}
