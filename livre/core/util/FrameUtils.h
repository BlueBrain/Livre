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

#ifndef _FrameUtils_h_
#define _FrameUtils_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/mathTypes.h>

namespace livre
{
/**
 * Class that takes care of the computation of a valid frame range and uses it
 * as a reference to compute valid frame numbers.
 */
class FrameUtils
{
public:
    /**
     * Constructor that takes a frame range and the boundaries for it
     * (e.g. data range) and creates an object that will use them to compute
     * valid frame numbers.
     * @param frameRange the specified frame range used as a reference to
     * construct the object
     * @param boundaries the range that will be used as a hard limit to compute
     * the final frame range used within the object
     */
    LIVRECORE_API FrameUtils( const Vector2ui& frameRange, const Vector2ui& boundaries );

    /**
     * Return the frame range stored in the object and used as a reference to
     * compute the frame numbers. Its value can be INVALID_FRAME_RANGE if
     * the frame range used at construction time is outside the boundaries
     * specified.
     * @return the frame range
     */
    LIVRECORE_API const Vector2ui& getFrameRange() const;

    /**
     * Compute the current frame number, based on the value used as input, as
     * well as the frame range stored in the object. Optionally, it can make the
     * returned value stick to the latest available.
     * @param frameNumber the value used as input to determine the frame number
     * @param latestAlways if true, return always the last frame number
     * available: (y - 1)
     * @return the current frame number. If the active frame range is
     * INVALID_FRAME_RANGE, return INVALID_FRAME
     */
    LIVRECORE_API uint32_t getCurrent( uint32_t frameNumber, bool latestAlways = false) const;

    /**
     * Compute the next frame number, based on the current frame number and the
     * delta used as input, as well as the frame range stored in the object.
     * If the potential frame number exceeds the specified frame range, it will
     * loop back to its other end (from the end to the beginning or vice versa,
     * depending on the sign of delta).
     * @param current the current frame number, used as a reference to compute
     * the next frame number
     * @param delta the value in which the frame number will be incremented.
     * It can also be negative
     * @return the next frame number. If the active frame range is
     * INVALID_FRAME_RANGE, return INVALID_FRAME
     */
    LIVRECORE_API uint32_t getNext( uint32_t current, int32_t delta ) const;

private:
    Vector2ui _clampToRange( const Vector2ui& frameRange,
                             const Vector2ui& boundaries );

    Vector2ui _frameRange;
};
}

#endif // _FrameUtils_h_
