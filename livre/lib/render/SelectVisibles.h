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

#ifndef _SelectVisibles_h_
#define _SelectVisibles_h_

#include <livre/lib/types.h>
#include <livre/core/visitor/RenderNodeVisitor.h>
#include <livre/core/dash/DashRenderNode.h>
#include <livre/core/dash/DashTree.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/maths/maths.h>
#include <livre/core/data/VolumeInformation.h>

//#define LIVRE_STATIC_DECOMPOSITION

namespace livre
{
/** Selects all visible rendering nodes */
class SelectVisibles : public livre::RenderNodeVisitor
{
public:
    SelectVisibles( DashTree& dashTree,
                    const Frustum& frustum,
                    const uint32_t windowHeight,
                    const float screenSpaceError,
                    const uint32_t minLOD,
                    const uint32_t maxLOD,
                    const Range& range )
    : RenderNodeVisitor( dashTree )
    , _frustum( frustum )
    , _windowHeight( windowHeight )
    , _screenSpaceError( screenSpaceError )
    , _minLOD( minLOD )
    , _maxLOD( maxLOD )
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
        renderNode.setLODVisible( false );
        if( !isInFrustum )
        {
            state.setVisitChild( false );
            return;
        }

        Vector3f vmin, vmax;
        const Plane& nearPlane = _frustum.getNearPlane();

        worldBox.computeNearFar( nearPlane, vmin, vmax );

        Vector4f hVmin = vmin;
        hVmin[ 3 ] = 1.0f;

        Vector4f hVmax = vmax;
        hVmax[ 3 ] = 1.0f;

        // The bounding box intersects the plane
        if( _frustum.getNearPlane().dot( hVmin ) < 0 ||
            _frustum.getNearPlane().dot( hVmax ) < 0 )
        {
            // Where eye direction intersects with near plane
            vmin = _frustum.getEyePos() - _frustum.getViewDir() * _frustum.nearPlane();
        }

        const Vector3f voxelBox = lodNode.getVoxelBox().getSize();
        const Vector3f worldSpacePerVoxel = worldBox.getSize() / voxelBox;

        bool isLODVisible = maths::isLODVisible( _frustum,
                                                  vmin,
                                                  worldSpacePerVoxel.find_min(),
                                                  _windowHeight,
                                                  _screenSpaceError );

        const VolumeInformation& volInfo =
                getDashTree().getDataSource().getVolumeInfo();
        const uint32_t depth = volInfo.rootNode.getDepth();
        isLODVisible = ( isLODVisible && lodNode.getRefLevel() >= _minLOD )
                       || ( lodNode.getRefLevel() == _maxLOD )
                       || ( lodNode.getRefLevel() == depth - 1 );

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

    const Frustum& _frustum;
    const uint32_t _windowHeight;
    const float _screenSpaceError;
    const uint32_t _minLOD;
    const uint32_t _maxLOD;
    const Range _range;
    DashRenderNodes _visibles;
};
}
#endif

