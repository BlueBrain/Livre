
/* Copyright (c) 2007-2015, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef _Node_h_
#define _Node_h_

#include <livre/core/Dash/DashContextTrait.h>
#include <livre/Lib/types.h>
#include <eq/node.h>

namespace livre
{

namespace detail
{
class Node;
}

/**
 * The Node class is a standard EQ abstraction for a process. It manages the data loaders
 * and keeps the data cache.
 */
class Node : public eq::Node, public DashContextTrait
{
public:

    /**
     * @param parent Parent config that owns the Node.
     */
    Node( eq::Config* parent );
    ~Node();

    /**
     * @return The raw data cache.
     */
    RawDataCache& getRawDataCache();

    /**
     * @return The texture data cache.
     */
    TextureDataCache& getTextureDataCache();

    /**
     * @return The dash tree.
     */
    DashTreePtr getDashTree( );

private:
    bool configInit( const eq::uint128_t& initId ) final;
    void frameStart(  const eq::uint128_t& frameId,
                      const uint32_t frameNumber ) final;
    bool configExit() final;

    detail::Node* _impl;
};

}

#endif //_Node_h_
