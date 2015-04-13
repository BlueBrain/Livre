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

#include <livre/Lib/Cache/RawDataObject.h>
#include <livre/core/Data/LODNode.h>

namespace livre
{

RawDataObject::RawDataObject( )
{
}

RawDataObject::RawDataObject( VolumeDataSourcePtr dataSourcePtr,
                              ConstLODNodePtr lodNodePtr )
    : dataSourcePtr_( dataSourcePtr )
    , data_( new NoMemoryUnit )
{
    lodNodePtr_ = lodNodePtr;

    // Level 0 is always unloadable
    if( lodNodePtr_->getRefLevel() == 0 )
        setUnloadable( false );
}

RawDataObject::~RawDataObject()
{
}

RawDataObject* RawDataObject::getEmptyPtr()
{
    static boost::shared_ptr< RawDataObject > data( new RawDataObject() );
    return data.get();
}

CacheId RawDataObject::getCacheID() const
{
    return lodNodePtr_->getNodeId().getId();
}

bool RawDataObject::operator==( const RawDataObject& data ) const
{
    return data.lodNodePtr_ == lodNodePtr_;
}

bool RawDataObject::isLoaded_( ) const
{
    return data_->getMemSize() > 0;
}

bool RawDataObject::isValid_( ) const
{
    return lodNodePtr_->isValid();
}

uint32_t RawDataObject::getDataSize() const
{
    if( !lodNodePtr_->isValid() )
        return 0;

    const Vector3i& voxSizeVec = lodNodePtr_->getVoxelBox( ).getDimension( );
    return voxSizeVec[0] * voxSizeVec[1] * voxSizeVec[2] *
           dataSourcePtr_->getVolumeInformation().compCount *
           dataSourcePtr_->getVolumeInformation().getBytesPerVoxel();
}

uint32_t RawDataObject::getCacheSize() const
{
    if( !lodNodePtr_->isValid() )
        return 0;

    return data_->getAllocSize();
}

ConstVolumeDataSourcePtr RawDataObject::getDataSource() const
{
    return dataSourcePtr_;
}

bool RawDataObject::load_( )
{
    data_ = dataSourcePtr_->getData( *lodNodePtr_ );
    return true;
}

void RawDataObject::unload_( )
{
    data_->release();
}


}
