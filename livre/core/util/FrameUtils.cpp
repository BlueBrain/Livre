/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Jafet Villafranca Diaz <jafet.villafrancadiaz@epfl.ch>
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

#include <livre/core/util/FrameUtils.h>

namespace livre
{

FrameUtils::FrameUtils( const Vector2ui& frameRange,
                        const Vector2ui& boundaries )
    : _frameRange( INVALID_FRAME_RANGE )
{
    // frame range is outside the boundaries
    if( frameRange[1] <= boundaries[0] || frameRange[0] >= boundaries[1] )
        return;

    _frameRange = _clampToRange( frameRange, boundaries );
}

const Vector2ui& FrameUtils::getFrameRange() const
{
    return _frameRange;
}

uint32_t FrameUtils::getCurrent( const uint32_t frameNumber,
                                 const bool latestAlways ) const
{
    if( _frameRange == INVALID_FRAME_RANGE )
        return INVALID_FRAME;

    const uint32_t lastFrame =  _frameRange[1] - 1;
    if( latestAlways )
        return lastFrame;

    const uint32_t currentFrame = frameNumber == INVALID_FRAME ? 0 : frameNumber;

    return std::min( std::max( _frameRange[0], currentFrame ), lastFrame );
}

uint32_t FrameUtils::getNext( uint32_t current, const int32_t delta ) const
{
    if( _frameRange == INVALID_FRAME_RANGE )
        return INVALID_FRAME;

    const uint32_t interval = _frameRange[1] - _frameRange[0];

    // If current is at the beginning and animation is reverse,
    // set current to the end
    if(( current == _frameRange[0] ) && ( delta < 0 ))
        current = _frameRange[1];

    return ( current - _frameRange[0] + delta ) % interval + _frameRange[0];
}

Vector2ui FrameUtils::_clampToRange( const Vector2ui& frameRange,
                                     const Vector2ui& boundaries )
{
    const uint32_t frameMin = std::max( frameRange[0], boundaries[0] );
    const uint32_t frameMax = std::min( frameRange[1], boundaries[1] );

    if( frameRange[0] < boundaries[0] || frameRange[1] > boundaries[1] )
        LBINFO << "Clamping the requested frame range to the boundaries. "
               << "Frame range used: [ " << frameMin << ", " << frameMax << " )"
               << std::endl;

    return Vector2ui( frameMin, frameMax );
}
}
