
/* Copyright (c) 2006-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
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

#include <livre/eq/settings/EqFrameSettings.h>

#include <co/dataOStream.h>
#include <co/dataIStream.h>

namespace livre
{

EqFrameSettings::EqFrameSettings()
    : _currentViewId( eq::uint128_t( 0 ),
                      [this](const eq::uint128_t&){ setDirty( DIRTY_ALL ); })
{
    _frameNumber = SignalledVariable< uint32_t >( _frameNumber.get(),
                                    [this](const uint32_t&){ setDirty( DIRTY_ALL ); });
    _screenShot =  SignalledVariable< uint32_t >( _screenShot.get(),
                                    [this](const uint32_t&){ setDirty( DIRTY_ALL ); });
    _statistics =  SignalledVariable< bool >( _statistics.get(),
                                    [this](const bool&){ setDirty( DIRTY_ALL ); });
    _info =  SignalledVariable< bool >( _info.get(),
                                        [this](const bool&){ setDirty( DIRTY_ALL ); });
    _grabFrame=  SignalledVariable< bool >( _grabFrame.get(),
                                            [this](const bool&){ setDirty( DIRTY_ALL ); });
}

void EqFrameSettings::setCurrentViewId( const eq::uint128_t &id )
{
    _currentViewId = id;
}

eq::uint128_t EqFrameSettings::getCurrentViewId() const
{
    return _currentViewId.get();
}

void EqFrameSettings::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );
    os << _currentViewId.get()
       << _frameNumber.get()
       << _screenShot.get()
       << _statistics.get()
       << _info.get()
       << _grabFrame.get();
}

void EqFrameSettings::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );
    is >> _currentViewId.get()
       >> _frameNumber.get()
       >> _screenShot.get()
       >> _statistics.get()
       >> _info.get()
       >> _grabFrame.get();
}
}
