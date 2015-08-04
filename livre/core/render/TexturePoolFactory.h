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

#ifndef _TexturePoolGroup_h_
#define _TexturePoolGroup_h_

#include <livre/core/api.h>
#include <livre/core/mathTypes.h>

namespace livre
{

/**
 * The TexturePoolFactory class is responsible for creating/providing texture pools for given OpenGL and
 * maximum block size parameters.
 */
class TexturePoolFactory
{
public:
    /**
     * @param internalFormat Internal OpenGL format for the texture, which defines the memory usage of a texture.
     */
    LIVRECORE_API explicit TexturePoolFactory( const int32_t internalFormat );

    /**
     * Generates/Retrieves a texture pool for the given format.
     * @param maxBlockSize The maximum voxel size of a texture block.
     * @param format  OpenGL format of the texture data kept in memory. Channel count etc.
     * @param gpuDataType Returns the OpenGL data type of the texture data.
     * @return The texture pool with given parameters.
     */
    LIVRECORE_API TexturePoolPtr findTexturePool( const Vector3i& maxBlockSize,
                                                  const uint32_t format,
                                                  const uint32_t gpuDataType );
private:

    const int32_t internalFormat_;
    TexturePools texturePools_;
};


}

#endif // _TexturePool_h_
