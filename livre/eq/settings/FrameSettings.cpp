
/* Copyright (c) 2006-2017, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <livre/eq/types.h>

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace livre
{
FrameSettings::FrameSettings()
{
    reset();
}

void FrameSettings::reset()
{
    frameNumber_ = INVALID_TIMESTEP;
    statistics_ = false;
    info_ = false;
    grabFrame_ = false;
    setDirty(DIRTY_ALL);
}

void FrameSettings::serialize(co::DataOStream& os, uint64_t)
{
    os << frameNumber_ << statistics_ << info_ << grabFrame_ << idle_;
}

void FrameSettings::deserialize(co::DataIStream& is, uint64_t)
{
    is >> frameNumber_ >> statistics_ >> info_ >> grabFrame_ >> idle_;
}

void FrameSettings::setFrameNumber(uint32_t frame)
{
    if (frameNumber_ == frame)
        return;

    frameNumber_ = frame;
    setDirty(DIRTY_ALL);
}

void FrameSettings::toggleStatistics()
{
    statistics_ = !statistics_;
    setDirty(DIRTY_ALL);
}

void FrameSettings::toggleInfo()
{
    info_ = !info_;
    setDirty(DIRTY_ALL);
}

bool FrameSettings::getShowInfo() const
{
    return info_;
}

bool FrameSettings::getStatistics() const
{
    return statistics_;
}

void FrameSettings::setGrabFrame(const bool setValue)
{
    if (grabFrame_ != setValue)
    {
        grabFrame_ = setValue;
        setDirty(DIRTY_ALL);
    }
}

bool FrameSettings::getGrabFrame() const
{
    return grabFrame_;
}

void FrameSettings::setIdle(const bool idle)
{
    if (idle == idle_)
        return;

    idle_ = idle;
    setDirty(DIRTY_ALL);
}

bool FrameSettings::isIdle() const
{
    return idle_;
}
}
