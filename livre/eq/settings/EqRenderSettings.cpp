
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

#include <livre/eq/settings/EqRenderSettings.h>

namespace livre
{

EqRenderSettings::EqRenderSettings()
{

    _colorMap =  SignalledVariable< lexis::render::ColorMap >( _colorMap.get(),
                                [this](const lexis::render::ColorMap&){ setDirty( DIRTY_ALL ); } );

    _clipPlanes = SignalledVariable< ClipPlanes >( _clipPlanes.get(),
                                [this](const ClipPlanes&){ setDirty( DIRTY_ALL ); } );


    _colorMap.get().registerDeserializedCallback( [this]
        { setDirty( DIRTY_COLORMAP ); });

    _clipPlanes.get().registerDeserializedCallback( [this]
        { setDirty( DIRTY_CLIPPLANES ); });

    _clipPlanes.get().clear();
}
void EqRenderSettings::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_COLORMAP )
        os << _colorMap.get();

    if( dirtyBits & DIRTY_DEPTH )
        os << _depth.get();

    if( dirtyBits & DIRTY_CLIPPLANES )
        os << _clipPlanes.get();

    co::Serializable::serialize( os, dirtyBits );
}

void EqRenderSettings::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_COLORMAP )
        is >> _colorMap.get();

    if( dirtyBits & DIRTY_DEPTH )
        is >> _depth.get();

    if( dirtyBits & DIRTY_CLIPPLANES )
        is >> _clipPlanes.get();

    co::Serializable::deserialize( is, dirtyBits );
}

}
