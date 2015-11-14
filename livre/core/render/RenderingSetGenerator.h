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

#ifndef _RenderingSetGenerator_h_
#define _RenderingSetGenerator_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/data/NodeId.h>

namespace livre
{

/**
 * The RenderingSetGenerator class is used to generate the list of nodes to be rendered.
 */
class RenderingSetGenerator
{
public:

    RenderingSetGenerator() {}

    LIVRECORE_API virtual ~RenderingSetGenerator() {}

    /**
     * Generates the rendering set according to the given frustum.
     * @param frameInfo keeps the frame information
     * @param allVisibleNodes list of all visible nodes
     */
    virtual void generateRenderingSet( FrameInfo& frameInfo,
                                       const NodeIds& allVisibleNodes ) = 0;
};

}

#endif // _RenderingSetGenerator_h_
