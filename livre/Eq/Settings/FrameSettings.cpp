
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 *                    2013, Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
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

#include <livre/Eq/Settings/FrameSettings.h>

namespace livre
{

FrameSettings::FrameSettings()
{
    reset();
}

void FrameSettings::reset()
{
    recording_ = false;
    statistics_ = false;
    help_ = false;
    grabFrame_= false;
    screenShot_ = 0;
    currentViewId_ = lunchbox::uint128_t( 0 );
    setDirty( DIRTY_ALL );
}

void FrameSettings::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );

    os << recording_
       << statistics_
       << help_
       << grabFrame_
       << screenShot_
       << currentViewId_;
}

void FrameSettings::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );

    is >> recording_
       >> statistics_
       >> help_
       >> grabFrame_
       >> screenShot_
       >> currentViewId_;
}

void FrameSettings::makeScreenshot()
{
    screenShot_++;
    setDirty( DIRTY_ALL );
}

void FrameSettings::toggleRecording()
{
    recording_ = !recording_;
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

bool FrameSettings::getRecording() const
{
    return recording_;
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
