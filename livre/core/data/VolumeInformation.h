/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _VolumeInformation_h_
#define _VolumeInformation_h_

#include <livre/core/api.h>
#include <livre/core/data/NodeId.h>

namespace livre
{

/** Voxel data type in a volume */
enum DataType
{
    DT_FLOAT,
    DT_UINT8,
    DT_UINT16,
    DT_UINT32,
    DT_INT8,
    DT_INT16,
    DT_INT32,
    DT_UNDEFINED
};

/** Properties of the volumetric data. */
struct VolumeInformation
{
    LIVRECORE_API VolumeInformation();

    /**
     * The endianness of the data.
     */
    bool isBigEndian;

    /**
     * The component count of voxel.
     */
    uint32_t compCount;

    /**
     * The data type of every component of voxel.
     */
    DataType dataType;

    /**
     * The overlap voxels between blocks.
     */
    Vector3ui overlap;

    /**
     * The maximum voxel size of a block.
     */
    Vector3ui maximumBlockSize;

    Vector3ui voxels; //!< The size (number of voxels) in each dimension

    /** The normalized size of the volume. */
    Vector3f worldSize;

    /** The transformation matrix between data space and livre space */
    Matrix4f dataToLivreTransform;

    /** The resolution in "data units"/voxel. Only valid for some datasources,
     * ( -1, -1, -1) otherwise.
     */
    Vector3f resolution;

    /**
     * The world space per voxel. Assumes the volume is isotropic.
     */
    float worldSpacePerVoxel;

    /**
     * The ratio between a meter and the unit used by the data.
     */
    float meterToDataUnitRatio;

    /**
      * Root node info
      * keeps level count, number of blocks, number of frames
      */
    RootNode rootNode;

    /** @return the number of bytes per element. */
    LIVRECORE_API size_t getBytesPerVoxel() const;

    /** The frame range for the data sources. If there are no frames,
      * [0,0) range is the default value. In streaming data sources
      * frame range can change over time.
      */
    Vector2ui frameRange;

    /** Optional description for end users. */
    std::string description;
};

}

#endif // _VolumeInformation_h_
