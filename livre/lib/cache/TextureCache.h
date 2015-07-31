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

#ifndef _TextureCache_h_
#define _TextureCache_h_

#include <livre/lib/types.h>
#include <livre/lib/cache/LRUCache.h>
#include <livre/core/render/TexturePoolFactory.h>

namespace livre
{

/**
 * TextureCache class handles the \see TextureObject memory management. \see TextureObject holds the information
 * for OpenGL reference and amount of data on the GPU.
 */
class TextureCache : public LRUCache
{
public:

    /**
     * @param internalTextureFormat Internal texture format of OpenGL, it defines the memory usage.
     */
    TextureCache( const int internalTextureFormat );

    /**
     * @param cacheID The cacheId of the node.
     * @return A valid cache object if id is valid or data source includes the information for cache id.
     * If there is not, the object is constructed.
     */
    TextureObject& getNodeTexture( const CacheId cacheID );

    /**
     * @param cacheID The cacheId of the node.
     * @return A valid cache object if id is valid or data source includes the information for cache id.
     */
    TextureObject& getNodeTexture( const CacheId cacheID ) const;

    /**
     * @param maxBlockSize Max block size of the texture.
     * @param format Format of the texture data.
     * @param gpuDataType Data type of the texture data.
     * @return A texture pool for specific type of texture.
     */
    TexturePoolPtr getTexturePool( const Vector3i& maxBlockSize,
                                   const uint32_t format,
                                   const uint32_t gpuDataType );

private:

    CacheObject *generateCacheObjectFromID_(const CacheId cacheID );
    TexturePoolFactory texturePoolFactory_;
};

}
#endif //_TextureCache_h_
