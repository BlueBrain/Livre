/**
 * Copyright (c) BBP/EPFL 2005-2015 Ahmet.Bilgili@epfl.ch
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

#ifndef _coSerialization_h_
#define _coSerialization_h_

#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <livre/core/data/VolumeInformation.h>
#include <lunchbox/bitOperation.h>


namespace livre
{

inline co::DataOStream& operator << ( co::DataOStream& os,
                               const VolumeInformation& info )
{
    os << info.isBigEndian << info.compCount << info.dataType << info.overlap
       << info.maximumBlockSize << info.voxels << info.worldSize
       << info.dataToLivreTransform << info.resolution
       << info.worldSpacePerVoxel << info.meterToDataUnitRatio
       << info.rootNode.getDepth() << info.rootNode.getBlockSize()
       << info.frameRange << info.description;
    return os;
}

inline co::DataIStream& operator >> ( co::DataIStream& is,
                                      VolumeInformation& info )
{
    uint32_t depth;
    Vector3ui blockSize;
    is >> info.isBigEndian >> info.compCount >> info.dataType >> info.overlap
       >> info.maximumBlockSize >> info.voxels >> info.worldSize
       >> info.dataToLivreTransform >> info.resolution
       >> info.worldSpacePerVoxel >> info.meterToDataUnitRatio >> depth
       >> blockSize >> info.frameRange >> info.description;
    info.rootNode = RootNode( depth, blockSize );
    return is;
}

}

namespace lunchbox
{

template<> inline void byteswap( vmml::vector< 3, unsigned int >& value )
{
    byteswap( value.x( ));
    byteswap( value.y( ));
    byteswap( value.z( ));
}

template<> inline void byteswap( livre::DataType& value )
{
    byteswap( reinterpret_cast< uint32_t& >( value ));
}

}

#endif // _coSerialization_h_
