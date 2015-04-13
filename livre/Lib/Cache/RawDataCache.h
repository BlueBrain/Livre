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

#ifndef _RawDataCache_h_
#define _RawDataCache_h_

#include <livre/Lib/types.h>

#include <livre/Lib/Cache/LRUCache.h>

namespace livre
{

/**
 * RawDataCache class handles the \see RawDataObject memory management. These objects hold the raw data
 * from the data source.
 */
class RawDataCache : public LRUCache
{
public:

     RawDataCache( );

    /**
     * Sets data source.
     * @param volumeDataSourcePtr Data source.
     */
    void setDataSource( VolumeDataSourcePtr volumeDataSourcePtr );

    /**
     * Get data source.
     * @param volumeDataSourcePtr Data source.
     */
    VolumeDataSourcePtr getDataSource( );

    /**
     * Get a const data source.
     * @param volumeDataSourcePtr Const data source.
     */
    ConstVolumeDataSourcePtr getDataSource( ) const;

    /**
     * @param cacheID The cacheId of the node.
     * @return A valid cache object if id is valid or data source includes the information for CacheId.
     * If there is not, the object is constructed.
     */
    RawDataObject& getNodeData( const CacheId cacheID );

    /**
     * @param cacheID The cacheId of the node.
     * @return A valid cache object if id is valid or data source includes the information for cache id.
     */
    RawDataObject& getNodeData( const CacheId cacheID ) const;

private:

    virtual CacheObject* generateCacheObjectFromID_( const CacheId cacheID );
    VolumeDataSourcePtr volumeDataSourcePtr_;
};

}
#endif // _RawDataCache_h_
