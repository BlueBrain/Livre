
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *               2013, Ahmet Bilgili  <ahmet.bilgili@epfl.ch>
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

#ifndef _RenderSettings_h_
#define _RenderSettings_h_

#include <livre/core/types.h>
#include <livre/core/data/SignalledVariable.h>
#include <livre/core/render/ClipPlanes.h>
#include <lexis/render/ColorMap.h>

namespace livre
{
class RenderSettings
{   
public:

    /** constructor. */
    RenderSettings();

    /**
     * Sets the color map.
     * @param cm is the color map
     */
    void setColorMap( const lexis::render::ColorMap& cm );

    /** Resets the color map to default values */
    void resetColorMap();

    /** @returns the color map. */
    lexis::render::ColorMap& getColorMap() { return _colorMap.get(); }
    const lexis::render::ColorMap& getColorMap() const { return _colorMap.get(); }

    /**
     * @brief Sets the clip planes.
     * @param clipPlanes the clip planes
     */
    void setClipPlanes( const ClipPlanes& clipPlanes );

    /**
     * @return Returns the clip planes.
     */
    ClipPlanes& getClipPlanes() { return _clipPlanes.get(); }
    const ClipPlanes& getClipPlanes() const { return _clipPlanes.get(); }

    /** @param depth Sets the maximum rendering depth. */
    void setMaxTreeDepth( const uint8_t depth );

    /** @return Returns the maximum rendering depth. */
    uint8_t getMaxTreeDepth( ) const;

    /**
     * Adjusts the quality.
     * @param delta The adjustment factor.
     */
    void adjustQuality( float delta );

    /** Increases the error. */
    void increaseError();

    /** Decreases the error. */
    void decreaseError();

protected:

    SignalledVariable< lexis::render::ColorMap > _colorMap;
    SignalledVariable< ClipPlanes >_clipPlanes;
    SignalledVariable< uint8_t > _depth;
};

}
#endif // _RenderSettings_h_
