
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

#include <eq/channel.h>

namespace livre
{

namespace detail
{
class Channel;
}

/**
 * The Channel class implements a view in opengl.
 */
class Channel : public eq::Channel
{
public:
    /**
     * @param parent Parent window.
     */
    Channel( eq::Window* parent );

    ~Channel();

private:
    bool configInit( const eq::uint128_t& initId ) final;
    bool configExit() final;

    void frameDraw( const eq::uint128_t& frameId ) final;
    void frameFinish( const eq::uint128_t&, const uint32_t ) final;
    void frameViewStart( const eq::uint128_t& frameId ) final;
    void frameViewFinish( const eq::uint128_t &frameID ) final;
    void frameAssemble( const eq::uint128_t&, const eq::Frames& ) final;
    void frameReadback( const eq::uint128_t&, const eq::Frames& ) final;

    detail::Channel* _impl;
};

}

#endif //_Channel_h_
