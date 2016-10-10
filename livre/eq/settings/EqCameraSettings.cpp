
/* Copyright (c) 2006-2016, Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
 *                          Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
 *                          Daniel.Nachbaur@epfl.ch
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

#include <livre/eq/settings/EqCameraSettings.h>

#include <co/co.h>
#include <eq/fabric/vmmlib.h>
#include <algorithm>

namespace livre
{

EqCameraSettings::EqCameraSettings()
{
    _modelview = SignalledVariable< Matrix4f >( Matrix4f(),
                                                [this]( const Matrix4f& mat )
                                                { _notifyChangedFunc( mat );
                                                  setDirty( DIRTY_ALL ); });
}

void EqCameraSettings::registerNotifyChanged(
        const std::function<void (const Matrix4f &)>& notifyChangedFunc )
{
    _notifyChangedFunc = notifyChangedFunc;
}

void EqCameraSettings::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );
    os << _modelview.get();
}
void EqCameraSettings::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );
    is >> _modelview.get();
}
}
