/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Stefan.Eilemann@epfl.ch
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

#ifndef LIVRE_SELECTVISIBLES_H
#define LIVRE_SELECTVISIBLES_H

#include <livre/lib/types.h>
#include <livre/core/visitor/RenderNodeVisitor.h>

//#define LIVRE_STATIC_DECOMPOSITION

namespace livre
{
/** Selects all visible rendering nodes */
class SelectVisibles : public livre::RenderNodeVisitor
{
public:
    SelectVisibles( DashTreePtr dashTree, const Frustum& frustum,
                    const uint32_t windowHeight, const float screenSpaceError,
                    const float worldSpacePerVoxel, const uint32_t volumeDepth,
                    const uint32_t minLOD, const uint32_t maxLOD,
                    const Range& range )
    : RenderNodeVisitor( dashTree )
    , _lodEvaluator( windowHeight, screenSpaceError, worldSpacePerVoxel,
                     minLOD, maxLOD )
    , _frustum( frustum )
    , _volumeDepth( volumeDepth )
    , _range( range )
    {}

    const DashRenderNodes& getVisibles() const { return _visibles; }

protected:
    void visitPre() final { _visibles.clear(); }

    void visit( DashRenderNode& renderNode, VisitState& state ) final
    {
        const LODNode& lodNode = renderNode.getLODNode();
        if( !lodNode.isValid( ))
            return;

        const Boxf& worldBox = lodNode.getWorldBox();
        const bool isInFrustum = _frustum.boxInFrustum( worldBox );
        renderNode.setInFrustum( isInFrustum );
        if( !isInFrustum )
        {
            renderNode.setLODVisible( false );
            state.setVisitChild( false );
            return;
        }

        Vector3f vmin, vmax;
        worldBox.computeNearFar( _frustum.getNearPlane(), vmin, vmax );

        const uint32_t lod =
            _lodEvaluator.getLODForPoint( _frustum, _volumeDepth, vmin );

        const bool isLODVisible = (lod <= lodNode.getNodeId().getLevel( ));
        if( isLODVisible )
            _visibles.push_back( renderNode );
        state.setVisitChild( !isLODVisible );
    }

    void visitPost() final
    {
        // Sort-last range selection:
#ifndef LIVRE_STATIC_DECOMPOSITION
        const size_t startIndex = _range[0] * _visibles.size();
        const size_t endIndex = _range[1] * _visibles.size();
#endif
        DashRenderNodes selected;

        for( size_t i = 0; i < _visibles.size(); ++i )
        {
#ifdef LIVRE_STATIC_DECOMPOSITION
            const Range& nodeRange =
                _visibles[i].getLODNode().getNodeId().getRange();
            const bool isInRange = nodeRange[1] > _range[0] &&
                                   nodeRange[1] <= _range[1];
#else
            const bool isInRange = i >= startIndex && i < endIndex;
#endif
            _visibles[i].setLODVisible( isInRange );
            _visibles[i].setInFrustum( isInRange );
            if( isInRange )
                selected.push_back( _visibles[i] );
        }
        _visibles.swap( selected );
    }

private:
    const ScreenSpaceLODEvaluator _lodEvaluator;
    const Frustum& _frustum;
    const uint32_t _volumeDepth;
    const Range _range;

    DashRenderNodes _visibles;
};
}
#endif
