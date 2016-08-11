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

#include <livre/core/data/VolumeInformation.h>
#include <livre/core/data/DataSourcePlugin.h>
#include <livre/core/data/LODNode.h>

namespace livre
{

VolumeInformation::VolumeInformation()
    : isBigEndian( false )
    , compCount( 1u )
    , dataType( DT_UINT8 )
    , overlap( 0u )
    , maximumBlockSize( 0u )
    , voxels( 256u )
    , worldSize( 0.0f )
    , resolution( Vector3f( -1.0f, -1.0f, -1.0f ))
    , worldSpacePerVoxel( 0.0f )
    , meterToDataUnitRatio( 1.0f )
    , frameRange( INVALID_FRAME_RANGE )
{}

size_t VolumeInformation::getBytesPerVoxel() const
{
    switch( dataType )
    {
    case DT_FLOAT64:
        return 8;
    case DT_FLOAT32:
    case DT_UINT32:
    case DT_INT32:
        return 4;
    case DT_UINT16:
    case DT_INT16:
        return 2;
    case DT_UINT8:
    case DT_INT8:
        return 1;
    default:
        return -1;
    }
}

}
