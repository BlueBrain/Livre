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

#include <livre/core/Cache/EmptyCacheObject.h>
#include <livre/Lib/Cache/RawDataCache.h>
#include <livre/Lib/Cache/RawDataObject.h>

namespace livre
{

RawDataCache::RawDataCache( )
{
    statisticsPtr_->setStatisticsName( "Data cache statistics" );
}

void RawDataCache::setDataSource(VolumeDataSourcePtr volumeDataSourcePtr )
{
    volumeDataSourcePtr_ = volumeDataSourcePtr;
}

VolumeDataSourcePtr RawDataCache::getDataSource()
{
    return volumeDataSourcePtr_;
}

ConstVolumeDataSourcePtr RawDataCache::getDataSource() const
{
    return volumeDataSourcePtr_;
}

CacheObject* RawDataCache::generateCacheObjectFromID_( const CacheId cacheID )
{
    ConstLODNodePtr lodNodePtr =
            volumeDataSourcePtr_->getNode( NodeId( cacheID ));

    if( !lodNodePtr->isValid() )
        return static_cast< CacheObject* >( RawDataObject::getEmptyPtr() );

    return new RawDataObject( volumeDataSourcePtr_, lodNodePtr );
}

RawDataObject &RawDataCache::getNodeData( const CacheId cacheID )
{
    if( cacheID == INVALID_CACHE_ID )
        return *RawDataObject::getEmptyPtr();

    return static_cast< RawDataObject& >( *getObjectFromCache_( cacheID ) );
}

RawDataObject &RawDataCache::getNodeData( const CacheId cacheID ) const
{
     if( cacheID == INVALID_CACHE_ID )
        return *RawDataObject::getEmptyPtr();

     return static_cast< RawDataObject& >( *getObjectFromCache_( cacheID ) );
}

}
