
/* Copyright (c) 2006-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <livre/Eq/types.h>
#include <eq/client/config.h> // base class

namespace livre
{
namespace detail { class Config; }

/**
 * Config class Runtime user events handling for the main application.
 */
class Config : public eq::Config
{
public:

    /**
     * @param parent Parent server object.
     */
    Config( eq::ServerPtr parent );

    /** @return The per-frame data. */
    FrameData& getFrameData();

    /** @return The per-frame data. */
    const FrameData& getFrameData() const;

    /**
     * mapFrameData Maps the init data.
     */
    void mapFrameData( const eq::uint128_t& initId );

    /**
     * Unmaps the init data.
     */
    void unmapFrameData( );

    /**
     * Resets camera.
     */
    void resetCamera( );

    /**
     * Initializes the config.
     * @return True if initalization is successful.
     */
    virtual bool init();

    /**
     * Is called on every frame start.
     * @return The id.
     */
    uint32_t startFrame();

    /**
     * Exits.
     * @return True if can exit cleanly.
     */
    virtual bool exit( );

    /**
     * switchCanvas_ Switches to next canvas.
     * @return True if operations succeeds.
     */
    bool switchCanvas_( );

    /**
     * Switches to next view.
     * @return True if operations succeeds.
     */
    bool switchView_( );

    /**
     * Switches to view canvas.
     * @return True if operations succeeds.
     */
    bool switchToViewCanvas_( const eq::uint128_t& viewID );

    /**
     * Switches the layout.
     * @param increment Increments the layout value.
     */
    void switchLayout_( const int32_t increment );

    /**
     * @brief handleEvents is called at the end of each frame.
     */
    void handleEvents() override;

private:

    virtual ~Config();
    bool handleEvent( const eq::ConfigEvent* event ) override;
    bool handleEvent( eq::EventICommand command ) override;

    bool registerFrameData_( );
    bool deregisterFrameData_();
    void initializeEvents_( );

    detail::Config* const _impl;
};

}

#endif // _Config_h_
