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

#include <livre/lib/api.h>
#include <livre/lib/types.h>

#include <livre/core/cache/Cache.h>

namespace livre
{

/**
 * TextureCache class handles the \see TextureObject memory management. \see TextureObject holds
 * the information for OpenGL reference and amount of data on the GPU.
 */
class TextureCache : public Cache
{
public:

    /**
     * @param dataCache Data cache.
     * @param maxMemBytes Maximum texture memory.
     */
    LIVRE_API TextureCache( DataCache& dataCache,
                            size_t maxMemBytes );

    ~TextureCache();

    /**
     * @return A texture pool for specific type of texture.
     */
    LIVRE_API TexturePool& getTexturePool() const;

    /**
     * @return the data cache
     */
    LIVRE_API DataCache& getDataCache();

    /**
     * @return the data cache
     */
    LIVRE_API const DataCache& getDataCache() const;

    /**
     * @return the OpenGL texture type
     */
    LIVRE_API uint32_t getTextureType() const;

private:

    CacheObject* _generate( const CacheId& cacheID );

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}
#endif //_TextureCache_h_
