
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

#ifndef _FrameSettings_h_
#define _FrameSettings_h_

#include <co/serializable.h> // base class

namespace livre
{
/** Enables/disables help, statistics, changes view. */
class FrameSettings : public co::Serializable
{
public:
    /**
     * FrameSettings constructor.
     */
    FrameSettings();

    /**
     * Resets the variables.
     */
    void reset();

    /**
     * Toggles the volume information.
     */
    void toggleInfo();

    /**
     * Toggles the statictics.
     */
    void toggleStatistics();

    /** Set the frame number of the current frame. */
    void setFrameNumber(uint32_t frame);

    /** @return the current frame number to render. */
    uint32_t getFrameNumber() const { return frameNumber_; }
    /**
     * @return Returns true if volume info is set.
     */
    bool getShowInfo() const;

    /**
     * @return Returns true if statistics is set.
     */
    bool getStatistics() const;

    /**
     * Enable/disable the frame grabbing for the current frame.
     * @param setValue the bolean value for enabling/disabling frame grabbing.
     */
    void setGrabFrame(const bool setValue);

    /**
     * @return true if frame grabbing is enabled for the current frame.
     */
    bool getGrabFrame() const;

    /**
     * Set idle rendering state aka the user stopped interacting: leads to
     * higher quality, slower interaction.
     */
    void setIdle(bool idle);

    /** @return true if idle rendering is active. */
    bool isIdle() const;

private:
    void serialize(co::DataOStream& os, const uint64_t dirtyBits) final;
    void deserialize(co::DataIStream& is, const uint64_t dirtyBits) final;

    uint32_t frameNumber_;
    bool statistics_;
    bool info_;
    bool grabFrame_;
    bool idle_{true};
};
}

#endif // _FrameInfo_h_
