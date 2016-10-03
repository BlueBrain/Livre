
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          David Steiner   <steiner@ifi.uzh.ch>
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

#ifndef _Config_h_
#define _Config_h_

#include <livre/eq/api.h>
#include <livre/eq/types.h>
#include <livre/eq/EventHandler.h> // base class
#include <eq/config.h> // CRTP base class

namespace livre
{
/** Drives a configuration, aka the main application loop. */
class Config : public EventHandler< eq::Config >
{
public:
    /**
     * @param parent Parent server object.
     */
    LIVREEQ_API Config( eq::ServerPtr parent );

    /** @return The per-frame data. */
    FrameData& getFrameData();

    /** @return The per-frame data. */
    const FrameData& getFrameData() const;

    /** @return the application configuration parameters */
    const ApplicationParameters& getApplicationParameters() const;

    /** @return the application configuration parameters */
    ApplicationParameters& getApplicationParameters();

    /**
     * mapFrameData Maps the init data.
     */
    void mapFrameData( const eq::uint128_t& initId );

    /**
     * Unmaps the init data.
     */
    void unmapFrameData();

    /**
     * Resets camera.
     */
    void resetCamera();

    /**
     * Initializes the config.
     * @return True if initalization is successful.
     */
    bool init( int argc, char** argv );

    /**
     * Exits the config.
     * @return True if exit is successful.
     */
    bool exit() final;

    /**
     * Trigger rendering of a new frame.
     * @return true if the frame is a valid frame.
     */
    bool frame();

    /** Schedule a redraw */
    void postRedraw();

    /** @return true if an event required a redraw. */
    bool needRedraw();

    /** Publish a serializable object */
    bool publish( const servus::Serializable& serializable );

    /**
     * Switches the layout.
     * @param increment Increments the layout value.
     */
    void switchLayout( const int32_t increment );

    void handleNetworkEvents();

    std::string renderJPEG();

    const VolumeInformation& getVolumeInformation() const;
    VolumeInformation& getVolumeInformation();

private:
    LIVREEQ_API virtual ~Config();

    bool _keepCurrentFrame( uint32_t fps ) const;

    class Impl;
    std::unique_ptr< Impl > _impl;
};

}

#endif // _Config_h_
