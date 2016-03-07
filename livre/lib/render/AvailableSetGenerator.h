/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _AvailableSetGenerator_h_
#define _AvailableSetGenerator_h_

#include <livre/lib/api.h>
#include <livre/lib/types.h>

namespace livre
{

/**
 * The AvailableSetGenerator class generates a rendering set according to the availability
 * of the textures.
 */
class AvailableSetGenerator
{
public:
    /**
     * @param textureCache is the cache for textures.
     */
    LIVRE_API explicit AvailableSetGenerator( const TextureCache& textureCache );
    LIVRE_API ~AvailableSetGenerator();

    /**
     * Generates the rendering set according to the given frustum.
     * @param frameInfo Keeps the frame information
     */
    LIVRE_API void generateRenderingSet( FrameInfo& frameInfo ) const;

private:

    struct Impl;
    std::unique_ptr< Impl > _impl;
};


}
#endif // _AvailableSetGenerator_h_
