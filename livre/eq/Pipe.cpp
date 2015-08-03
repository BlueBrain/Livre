
/* Copyright (c) 2012-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          David Steiner   <steiner@ifi.uzh.ch>
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

#include <livre/eq/Pipe.h>
#include <livre/eq/Config.h>
#include <livre/eq/FrameData.h>

namespace livre
{

namespace detail
{

class Pipe
{
public:
    explicit Pipe( livre::Pipe* pipe )
        : _pipe( pipe )
        , _frameDataPtr( new FrameData( ))
    {}

    bool mapFrameData( const eq::uint128_t& initId )
    {
        livre::Config* config = static_cast< livre::Config* >( _pipe->getConfig( ));
        _frameDataPtr->initialize( config );
        if( !_frameDataPtr->map( config, initId ))
            return false;
        _frameDataPtr->mapObjects();
        return true;
    }

    void unmapFrameData()
    {
        _frameDataPtr->unmapObjects();
        _frameDataPtr->unmap( static_cast< livre::Config* >( _pipe->getConfig( )));
    }

    void frameStart( const eq::uint128_t& frameId )
    {
        _frameDataPtr->sync( frameId );
    }

    bool configInit( const eq::uint128_t& initId )
    {
        return mapFrameData( initId );
    }

    void configExit()
    {
        unmapFrameData();
    }

    livre::Pipe* const _pipe;
    FrameDataPtr _frameDataPtr;
};

}

Pipe::Pipe( eq::Node* parent )
    : eq::Pipe( parent )
    , _impl( new detail::Pipe( this ))
{
}

Pipe::~Pipe( )
{
    delete _impl;
}

void Pipe::frameStart( const eq::uint128_t& frameId, const uint32_t frameNumber )
{
    _impl->frameStart( frameId );
    eq::Pipe::frameStart( frameId, frameNumber );
}

bool Pipe::configInit( const eq::uint128_t& initId )
{
    if( !eq::Pipe::configInit( initId ))
        return false;

    return _impl->configInit( initId );
}

bool Pipe::configExit()
{
    _impl->configExit();
    return eq::Pipe::configExit();
}

ConstFrameDataPtr Pipe::getFrameData() const
{
    return _impl->_frameDataPtr;
}

}
