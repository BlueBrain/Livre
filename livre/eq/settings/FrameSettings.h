
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

#ifndef _FrameSettings_h_
#define _FrameSettings_h_

#include <co/serializable.h>

#include <livre/lib/types.h>
#include <livre/core/mathTypes.h>

#include <livre/eq/Client.h>

namespace livre
{

/**
 * The FrameSettings class enables/disables help, statistics, recording. Changes view and also toogles the
 * screen shot.
 */
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
     * Toggles the help.
     */
    void toggleHelp();

    /**
     * Toggles the recording.
     */
    void toggleRecording();

    /**
     * Toggles the statictics.
     */
    void toggleStatistics();

    /** Set the frame number of the current frame. */
    void setFrameNumber( uint32_t frame );

    /** @return the current frame number to render. */
    uint32_t getFrameNumber() const { return frameNumber_; }

    /** Set the frame range */
    void setFrameRange( const Vector2ui& frameRange );

    /** @return the current frameRange */
    const Vector2ui& getFrameRange() const { return frameRange_; }

    /**
     * Screen shots the current frame.
     */
    void makeScreenshot();

    /**
     * @return Returns the screen shot number.
     */
    uint32_t getScreenshotNumber() const;

    /**
     * Set the current view id.
     * @param id The view id.
     */
    void setCurrentViewId( const eq::uint128_t& id );

    /**
     * @return Returns true if help is set.
     */
    bool getShowHelp() const;

    /**
     * @return Returns true if recording is set.
     */
    bool getRecording() const;

    /**
     * @return Returns true if statistics is set.
     */
    bool getStatistics() const;

    /**
     * @return Returns the current view id.
     */
    eq::uint128_t getCurrentViewId() const;

    /**
     * Enable/disable the frame grabbing for the current frame.
     * @param setValue the bolean value for enabling/disabling frame grabbing.
     */
    void setGrabFrame( const bool setValue );

    /**
     * @return true if frame grabbing is enabled for the current frame.
     */
    bool getGrabFrame() const;

private:

    virtual void serialize( co::DataOStream& os, const uint64_t dirtyBits );
    virtual void deserialize( co::DataIStream& is, const uint64_t dirtyBits );

    eq::uint128_t currentViewId_;
    uint32_t frameNumber_;
    Vector2ui frameRange_;
    uint32_t screenShot_;
    bool recording_;
    bool statistics_;
    bool help_;
    bool grabFrame_;
};


}

#endif // _FrameInfo_h_
