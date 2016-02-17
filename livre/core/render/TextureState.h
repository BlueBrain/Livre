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

#ifndef _TextureState_h_
#define _TextureState_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/mathTypes.h>

namespace livre
{

/**
 * The TextureState struct is a holder for the texture information such as OpenGL texture id, coordinates, etc.
 */
struct TextureState
{
public:
    LIVRECORE_API TextureState();

    /**
     * OpenGL bind() the texture.
     */
    LIVRECORE_API void bind() const;

    Vector3f textureCoordsMin; //!< Minimum texture coordinates in the maximum texture block in \see TexturePool.
    Vector3f textureCoordsMax; //!< Maximum texture coordinates in the maximum texture block in \see TexturePool.
    Vector3f textureSize; //!< The texture size.

    uint32_t textureId; //!< The OpenGL texture id.
};

}

#endif // _TextureState_h_
