
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

#ifndef _FrameSettings_h_
#define _FrameSettings_h_

#include <co/serializable.h>

#include <livre/Lib/types.h>
#include <livre/core/mathTypes.h>

#include <livre/Eq/Client.h>

namespace livre
{

/**
 * @brief The FrameSettings class enables/disables help, statistics, recording. Changes view and also toogles the
 * screen shot.
 */
class FrameSettings : public co::Serializable
{
public:

    /**
     * @brief FrameSettings constructor.
     */
    FrameSettings();

    /**
     * @brief reset Resets the variables.
     */
    void reset();

    /**
     * @brief toggleHelp Toggles the help.
     */
    void toggleHelp();

    /**
     * @brief toggleRecording Toggles the recording.
     */
    void toggleRecording();

    /**
     * @brief toggleStatistics Toggles the statictics.
     */
    void toggleStatistics();

    /**
     * @brief makeScreenshot Screen shots the current frame.
     */
    void makeScreenshot();

    /**
     * @brief setCurrentViewId Set the current view id.
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
     * @return Returns the screen shot number.
     */
    uint32_t getScreenshotNumber() const;

    /**
     * @return Returns the current view id.
     */
    eq::uint128_t getCurrentViewId() const;

    /**
     * @brief setGrabFrame enable/disable the frame grabbing for the current frame.
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

    bool recording_;
    bool statistics_;
    bool help_;
    uint32_t screenShot_;
    eq::uint128_t currentViewId_;
    bool grabFrame_;
};


}

#endif // _FrameInfo_h_
