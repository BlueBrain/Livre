/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/core/mathTypes.h>
#include <livre/core/data/NodeId.h>

namespace livre
{

/**
 * The DataType enum is used for defining the data type in volumetric data.
 */
enum DataType
{
    DT_FLOAT32,
    DT_UINT8,
    DT_UINT16,
    DT_UINT32,
    DT_FLOAT64,
    DT_INT8,
    DT_INT16,
    DT_INT32,
    DT_UNDEFINED
};

/** Properties of the volumetric data. */
struct VolumeInformation
{
    VolumeInformation( VolumeDataSourcePlugin* dataSource = 0 );

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

    /**
     * The minimum position of a volume.
     */
    Vector3f minPos;

    /**
     * The maximum position of a volume.
     */
    Vector3f maxPos;

    Vector3ui voxels; //!< The size (number of voxels) in each dimension

    /** The normalized size of the volume. */
    Vector3f worldSize;

    /** Real-world size of the volume in meters. */
    Boxf boundingBox;

    /**
     * The world space per voxel. Assumes the volume is isotropic.
     */
    float worldSpacePerVoxel;

    /**
      * Root node info
      * keeps level count, number of blocks, number of frames
      */
    RootNode rootNode;

    /** @return the number of bytes per element. */
    size_t getBytesPerVoxel() const;

    /** @return the frame range for the data source in [start,end) format.
      * If there is no frames, [0,0) range is returned. In streaming data sources
      * frame range can change over time.
      */
    Vector2ui getFrameRange() const;

private:

    mutable VolumeDataSourcePlugin* _dataSource;
};

}

#endif // _VolumeInformation_h_
