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

#include <livre/core/Render/TextureState.h>
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
    int32_t getInternalFormat() const;

    /**
     * @return The OpenGL format of the texture data kept in memory. Channel count etc.
     */
    uint32_t getGPUDataType() const;

    /**
     * @return The OpenGL data type of the texture data.
     */
    uint32_t getFormat() const;

    /**
     * @return The maximum texture block size in voxels.
     */
    const Vector3i& getMaxBlockSize( ) const;

   /**
     * Generates / uses a preallocated a 3D OpenGL texture based on OpenGL parameters.
     * @param textureState The destination state is filled with needed information.
     */
    void generateTexture( TextureStatePtr textureState );

    /**
     * Releases a texture slot.
     * @param textureState The destination state is filled with needed information.
     */
    void releaseTexture( TextureStatePtr textureState );

private:

     TexturePool( const Vector3i& maxBlockSize,
                  const int internalFormat,
                  const uint32_t format,
                  const uint32_t gpuDataType );

     std::vector< uint32_t > textureStack_;

     const Vector3i maxBlockSize_;

     const int32_t internalFormat_;
     const uint32_t format_;
     const uint32_t gpuDataType_;
};


}

#endif // _TexturePool_h_
