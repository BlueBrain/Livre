
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

#include <livre/core/types.h>
#include <livre/core/mathTypes.h>
#include <livre/core/data/SignalledVariable.h>

namespace livre
{

/**
 * The FrameSettings class enables/disables info, statistics. Changes view and also toogles the
 * screen shot.
 */
class FrameSettings
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
    void setFrameNumber( uint32_t frame );

    /** @return the current frame number to render. */
    uint32_t getFrameNumber() const;

    /**
     * Screen shots the current frame.
     */
    void makeScreenshot();

    /**
     * @return Returns the screen shot number.
     */
    uint32_t getScreenshotNumber() const;

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
    void setGrabFrame( const bool setValue );

    /**
     * @return true if frame grabbing is enabled for the current frame.
     */
    bool getGrabFrame() const;

protected:

    SignalledVariable< uint32_t > _frameNumber;
    SignalledVariable< uint32_t > _screenShot;
    SignalledVariable< bool > _statistics;
    SignalledVariable< bool > _info;
    SignalledVariable< bool > _grabFrame;
};
}
#endif // _FrameSettings_h_
