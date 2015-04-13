
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

#ifndef _Channel_h_
#define _Channel_h_

#include <eq/client/channel.h>
#include <eq/client/frame.h>
#include <eq/fabric/range.h>

#include <livre/core/types.h>
#include <livre/core/Render/Frustum.h>

#include <livre/Eq/eqTypes.h>
#include <livre/Eq/types.h>

#include <livre/Eq/FrameGrabber.h>

namespace livre
{

/**
 * The Channel class implements view in opengl.
 */
class Channel : public eq::Channel
{
public:

    /**
     * @param parent Parent window.
     */
    Channel( eq::Window* parent );

    virtual ~Channel();

    /**
     * Computes the frustum.
     * @param frustum Frustum is initialized.
     */
    const Frustum& initializeLivreFrustum();

    /**
     * @return The \see GLWidget.
     */
    GLWidgetPtr getGLWidget();

private:

    void applyCamera_( );

    void initializeFrame_();

    void prepareFramesAndSetPvp_( const eq::Frames& frames,
                                  eq::Frames& dbFrames,
                                  eq::PixelViewport& coveredPVP,
                                  eq::Zoom& zoom );

    void composeFrames_(  const eq::PixelViewport& coveredPVP,
                          const eq::Zoom& zoom,
                          eq::FrameDataPtr data,
                          eq::Frames& dbFrames );

    void clearViewport_( const eq::PixelViewport &pvp );
    void initializeRenderer_( );
    void initializeGLContextAndStartUploaders_( );
    ConstFrameDataPtr getFrameData_( ) const;

    bool configInit( const eq::uint128_t& initId ) override;
    bool configExit() override;

    void frameDraw( const eq::uint128_t& frameId ) override;
    void frameFinish( const eq::uint128_t&, const uint32_t ) override;
    void frameViewStart( const uint128_t& frameId ) override;
    void frameViewFinish( const eq::uint128_t &frameID ) override;
    void frameAssemble( const eq::uint128_t&, const eq::Frames& ) override;
    void frameReadback( const eq::uint128_t&, const eq::Frames& ) override;
    void frameClear( const eq::uint128_t& frameId ) override;

    eq::Range drawRange_;
    eq::Frame frame_;
    Frustum currentFrustum_;
    ViewPtr renderViewPtr_;
    GLWidgetPtr glWidgetPtr_;
    FrameGrabber frameGrabber_;
};

}

#endif //_Channel_h_
