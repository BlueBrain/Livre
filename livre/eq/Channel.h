
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

#include <livre/eq/api.h>
#include <livre/core/types.h>

#include <eq/channel.h>

namespace livre
{

/** A view in OpenGL. */
class Channel : public eq::Channel
{
public:
    LIVREEQ_API Channel( eq::Window* parent );
    LIVREEQ_API ~Channel();

    struct Impl;

private:
    bool configInit( const eq::uint128_t& initId ) final;
    bool configExit() final;

    void frameStart( const eq::uint128_t&, const uint32_t ) final;
    void frameDraw( const eq::uint128_t& frameId ) final;
    void frameViewStart( const eq::uint128_t& frameId ) final;
    void frameViewFinish( const eq::uint128_t &frameID ) final;
    void frameAssemble( const eq::uint128_t&, const eq::Frames& ) final;
    void frameReadback( const eq::uint128_t&, const eq::Frames& ) final;
    std::string getDumpImageFileName() const final;

    std::unique_ptr< Impl >  _impl;
};

}

#endif //_Channel_h_
