/* Copyright (c) 2016, EPFL/Blue Brain Project
 *                     Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#ifndef _BoundingAxis_h_
#define _BoundingAxis_h_

#include <livre/core/data/VolumeInformation.h>

namespace livre
{
/**
 * The BoundingAxis class create and redner bounding axis of a volume
 */
class BoundingAxis
{
    struct BoundingAxisData
    {
        VolumeInformation volInfo;
        Floats vertices;
        Floats normals;
        Floats normals2;
        Floats colors;
        UInt8s types;
        float tickDistance;
        float factor;
    };

public:
    BoundingAxis(const VolumeInformation& volInfo);

    ~BoundingAxis();

    /**
     * Draw the bounding axises.
     */
    void draw();

private:
    void _createTicks(BoundingAxisData& bbAxisData);

    void _createAxisTicks(const Vector3f& start, const Vector3f& end,
                          const Vector3f& normal, const Vector3f& normal2,
                          bool flipTick, BoundingAxisData& bbAxisData);

    void _sendDataToGPU(const BoundingAxisData& bbAxisData);

    const Vector2f _computeRange(const VolumeInformation& volInfo,
                                 const size_t axis) const;

    uint32_t _axisBufferId;
    size_t _nVertices;

    uint32_t _typeBufferId;
};
}
#endif // _BoundingAxis_h_
