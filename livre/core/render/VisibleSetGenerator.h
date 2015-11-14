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

#ifndef _VisibleSetGenerator_h_
#define _VisibleSetGenerator_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/data/NodeId.h>

namespace livre
{

/**
 * The Visible class is used to generate the visible list of nodes.
 */
class VisibleSetGenerator
{
public:

    VisibleSetGenerator();

    LIVRECORE_API ~VisibleSetGenerator();

    /**
     * Generates the rendering set according to the given frustum.
     * @param lodEvaluator LOD selection algorithm
     * @param frustum view frustum
     * @param dataSource data source
     * @param frameId frame id
     * @param list of nodes needed for rendering the volume
     * @param nodeId to start from
     */
     void generateVisibleSet( const PixelViewport& viewport,
                              const Frustum& frustum,
                              const LODEvaluator& lodEvaluator,
                              const VolumeDataSource& dataSource,
                              uint32_t frame,
                              uint32_t minLOD,
                              uint32_t maxLOD,
                              NodeIds& allVisibleNodes,
                              const NodeId& nodeId = NodeId( ));
private:

     struct Impl;
     std::unique_ptr<Impl> _impl;
};

}

#endif // _VisibleSetGenerator_h_
