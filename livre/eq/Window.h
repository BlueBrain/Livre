/* Copyright (c) 2007-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#ifndef _Window_h_
#define _Window_h_

#include <eq/window.h>

#include <livre/eq/api.h>
#include <livre/eq/types.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * The Window class represents a window in the rendering system.
 */
class Window : public eq::Window
{
public:
    /** @param parent The pipe that owns the window. */
    LIVREEQ_API Window( eq::Pipe* parent );

    /** Commits changes. */
    void commit();

    /**
     * Applies all the changes to the context.
     * @param wait if true blocks until changes are applied to the current context
     * @return true if any change is applied to the current context
     */
    bool apply( bool wait);

    /** @return The texture data cache. */
    const TextureCache& getTextureCache() const;

private:

    bool configInit( const eq::uint128_t& initId ) final;
    bool configExit() final;
    bool configInitGL( const eq::uint128_t& initId ) final;
    bool configExitGL() final;
    void frameStart( const eq::uint128_t& frameID,
                     const uint32_t frameNumber ) final;

    struct Impl;
    std::unique_ptr< Impl > _impl;
};

}

#endif //_Window_h_
