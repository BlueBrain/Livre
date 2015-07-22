/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#ifndef _TextureDataCache_h_
#define _TextureDataCache_h_

#include <livre/Lib/types.h>
#include <livre/Lib/Cache/LRUCache.h>

namespace livre
{

/**
 * The TextureDataCache class stores the quantized/modified data for GPU
 * uploading. Therefore it decreases the amount of processing while data is
 * dropped from GPU and re-uploaded to GPU.
 */
class TextureDataCache : public LRUCache
{
public:

    /**
     * @param volumeDataSource The source where the volume data should be loaded from
     * @param type The type of the data for the GPU.
     */
    TextureDataCache( VolumeDataSourcePtr volumeDataSource, uint32_t type );

    /**
     * @param cacheID The cacheId of the node.
     * @return A valid cache object if id is valid or data source includes the information for cache id.
     * If there is not, the object is constructed.
     */
    TextureDataObject& getNodeTextureData( const CacheId cacheID ) const;

    /**
     * @param cacheID The cacheId of the node.
     * @return A valid cache object if id is valid or data source includes the information for cache id.
     */
    TextureDataObject& getNodeTextureData( const CacheId cacheID );

    /** @return the data source. */
    VolumeDataSourcePtr getDataSource();

    /**
     * @return The GPU data type.
     */
    uint32_t getTextureType( ) const { return type_; }

private:

    CacheObject *generateCacheObjectFromID_( const CacheId cacheID );

    VolumeDataSourcePtr volumeDataSourcePtr_;
    const uint32_t type_;
};

}
#endif //_LODTextureDataCache_h_
