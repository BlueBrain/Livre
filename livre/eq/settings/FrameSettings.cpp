
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

#include <livre/eq/settings/FrameSettings.h>

#include <co/dataOStream.h>
#include <co/dataIStream.h>

namespace livre
{

FrameSettings::FrameSettings()
{
    reset();
}

void FrameSettings::reset()
{
    currentViewId_ = lunchbox::uint128_t( 0 );
    frameNumber_ = INVALID_TIMESTEP;
    screenShot_ = 0;
    statistics_ = false;
    help_ = false;
    grabFrame_= false;
    setDirty( DIRTY_ALL );
}

void FrameSettings::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );
    os << currentViewId_ << frameNumber_ << screenShot_ << statistics_ << help_
       << grabFrame_;
}

void FrameSettings::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );
    is >> currentViewId_ >> frameNumber_ >> screenShot_ >> statistics_ >> help_
       >> grabFrame_;
}

void FrameSettings::setFrameNumber( uint32_t frame )
{
    if( frameNumber_ == frame )
        return;

    frameNumber_ = frame;
    setDirty( DIRTY_ALL );
}

void FrameSettings::makeScreenshot()
{
    screenShot_++;
    setDirty( DIRTY_ALL );
}

void FrameSettings::toggleStatistics()
{
    statistics_ = !statistics_;
    setDirty( DIRTY_ALL );
}

void FrameSettings::toggleHelp()
{
    help_ = !help_;
    setDirty( DIRTY_ALL );
}

bool FrameSettings::getShowHelp() const
{
    return help_;
}

bool FrameSettings::getStatistics() const
{
    return statistics_;
}

uint32_t FrameSettings::getScreenshotNumber() const
{
    return screenShot_;
}

void FrameSettings::setCurrentViewId( const eq::uint128_t &id )
{
    currentViewId_ = id;
    setDirty( DIRTY_ALL );
}

eq::uint128_t FrameSettings::getCurrentViewId() const
{
    return currentViewId_;
}

void FrameSettings::setGrabFrame( const bool setValue )
{
    if( grabFrame_ != setValue )
    {
        grabFrame_ = setValue;
        setDirty( DIRTY_ALL );
    }
}

bool FrameSettings::getGrabFrame() const
{
    return grabFrame_;
}

}
