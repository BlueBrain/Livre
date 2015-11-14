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

#ifndef _TexturePool_h_
#define _TexturePool_h_

#include <livre/core/api.h>
#include <livre/core/render/TextureState.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * The TexturePool class is responsible for allocating texture slots and copying textures into the texture slots.
 * The methods are not thread safe, the class should only be used by a single thread.
 */
class TexturePool
{
    friend class TexturePoolFactory;

public:

    /**
     * @return The OpenGL GPU internal format of the texture data.
     */
    LIVRECORE_API int32_t getInternalFormat() const;

    /**
     * @return The OpenGL format of the texture data kept in memory. Channel count etc.
     */
    LIVRECORE_API uint32_t getGPUDataType() const;

    /**
     * @return The OpenGL data type of the texture data.
     */
    LIVRECORE_API uint32_t getFormat() const;

    /**
     * @return The maximum texture block size in voxels.
     */
    LIVRECORE_API const Vector3i& getMaxBlockSize( ) const;

   /**
     * Generates / uses a preallocated a 3D OpenGL texture based on OpenGL parameters.
     * @param textureState The destination state is filled with needed information.
     */
    LIVRECORE_API void generateTexture( TextureStatePtr textureState );

    /**
     * Releases a texture slot.
     * @param textureState The destination state is filled with needed information.
     */
    LIVRECORE_API void releaseTexture( TextureStatePtr textureState );

    ~TexturePool();

private:
    TexturePool( const Vector3i& maxBlockSize,
                 const int internalFormat,
                 const uint32_t format,
                 const uint32_t gpuDataType );

    struct Impl;
    std::unique_ptr<Impl> _impl;
};


}

#endif // _TexturePool_h_
