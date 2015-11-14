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

#include <livre/core/render/RenderBrick.h>
#include <livre/core/render/View.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/FrameInfo.h>
#include <livre/core/render/LODEvaluator.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/data/VolumeInformation.h>
#include <livre/core/visitor/RenderNodeVisitor.h>
#include <livre/core/visitor/VisitState.h>
#include <livre/core/render/VisibleSetGenerator.h>
#include <livre/core/visitor/DFSTraversal.h>
#include <livre/core/visitor/CollectionTraversal.h>
#include <livre/lib/cache/TextureObject.h>
#include <livre/lib/cache/TextureCache.h>

namespace livre
{

class SelectVisibles : public livre::RenderNodeVisitor
{
public:

    SelectVisibles( const PixelViewport& viewport,
                    const Frustum& frustum,
                    const LODEvaluator& lodEvaluator,
                    const VolumeDataSource& dataSource,
                    uint32_t minLOD,
                    uint32_t maxLOD,
                    NodeIds& allVisibleNodes )

        : RenderNodeVisitor( dataSource )
        , _viewport( viewport )
        , _frustum( frustum )
        , _lodEvaluator( lodEvaluator )
        , _volInfo( dataSource.getVolumeInformation( ))
        , _minLOD( minLOD )
        , _maxLOD( maxLOD )
        , _allVisibleNodes( allVisibleNodes )
    {

    }

    void visit( const LODNode& lodNode, VisitState& state )
    {
        if( !lodNode.isValid( ))
            return;

        const Boxf& worldBox = lodNode.getWorldBox();

        const bool isInFrustum = _frustum.boxInFrustum( worldBox );
        if( !isInFrustum )
        {
            state.setVisitChild( false );
            return;
        }

        const Plane& nearPlane = _frustum.getWPlane( PL_NEAR );
        Vector3f vmin, vmax;
        nearPlane.getNearFarPoints( worldBox, vmin, vmax );

        const uint32_t lod = _lodEvaluator.getLODForPoint( vmin,
                                                          _volInfo,
                                                          _viewport,
                                                          _frustum,
                                                          _minLOD,
                                                          _maxLOD );

        const bool isLODVisible = (lod <= lodNode.getNodeId().getLevel( ));
        if( isLODVisible )
            _allVisibleNodes.push_back( lodNode.getNodeId( ));

        state.setVisitChild( !isLODVisible );
    }

    PixelViewport _viewport;
    Frustum _frustum;
    const LODEvaluator& _lodEvaluator;
    const VolumeInformation& _volInfo;
    uint32_t _minLOD;
    uint32_t _maxLOD;
    NodeIds& _allVisibleNodes;
};

struct VisibleSetGenerator::Impl
{
    Impl() {}

    void generateVisibleSet( const PixelViewport& viewport,
                             const Frustum& frustum,
                             const LODEvaluator& lodEvaluator,
                             const VolumeDataSource& dataSource,
                             uint32_t frame,
                             uint32_t minLOD,
                             uint32_t maxLOD,
                             NodeIds& allVisibleNodes,
                             const NodeId& nodeId )
    {
        SelectVisibles visibleSelector( viewport,
                                        frustum,
                                        lodEvaluator,
                                        dataSource,
                                        minLOD,
                                        maxLOD,
                                        allVisibleNodes );
        DFSTraversal dfsTraverser_;
        if( nodeId.isValid( ))
        {
            dfsTraverser_.traverse( dataSource.getVolumeInformation().rootNode,
                                    nodeId,
                                    visibleSelector );
        }
        else
        {
            dfsTraverser_.traverse( dataSource.getVolumeInformation().rootNode,
                                    visibleSelector,
                                    frame );
        }
    }
};


VisibleSetGenerator::VisibleSetGenerator()
    : _impl( new VisibleSetGenerator::Impl())
{
}

VisibleSetGenerator::~VisibleSetGenerator()
{
    
}

void VisibleSetGenerator::generateVisibleSet( const PixelViewport& viewport,
                                              const Frustum& frustum,
                                              const LODEvaluator& lodEvaluator,
                                              const VolumeDataSource& dataSource,
                                              uint32_t frame,
                                              uint32_t minLOD,
                                              uint32_t maxLOD,
                                              NodeIds& allVisibleNodes,
                                              const NodeId& nodeId )
{
    _impl->generateVisibleSet( viewport,
                               frustum,
                               lodEvaluator,
                               dataSource,
                               frame,
                               minLOD,
                               maxLOD,
                               allVisibleNodes,
                               nodeId );
}

}
