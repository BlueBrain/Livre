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

#include <livre/core/settings/FrameSettings.h>

namespace livre
{
FrameSettings::FrameSettings()
    : _frameNumber( INVALID_TIMESTEP )
    , _screenShot( 0 )
    , _statistics( 0 )
    , _info( false )
    , _grabFrame( false )
{}

void FrameSettings::setFrameNumber( const uint32_t frame )
{
    if( _frameNumber.get() == frame )
        return;

    _frameNumber = frame;
}

uint32_t FrameSettings::getFrameNumber() const
{
    return _frameNumber.get();
}

void FrameSettings::makeScreenshot()
{
    _screenShot= (_screenShot.get())++;
}

void FrameSettings::toggleStatistics()
{
    _statistics = !_statistics.get();
}

void FrameSettings::toggleInfo()
{
    _info = !_info.get();
}

bool FrameSettings::getShowInfo() const
{
    return _info.get();
}

bool FrameSettings::getStatistics() const
{
    return _statistics.get();
}

uint32_t FrameSettings::getScreenshotNumber() const
{
    return _screenShot.get();
}

void FrameSettings::setGrabFrame( const bool setValue )
{
    if( _grabFrame.get() != setValue )
        _grabFrame = setValue;
}

bool FrameSettings::getGrabFrame() const
{
    return _grabFrame.get();
}
}

