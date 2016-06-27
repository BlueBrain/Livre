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
#include <livre/core/data/LODNode.h>
#include <livre/core/data/VolumeInformation.h>
#include <livre/core/visitor/DataSourceVisitor.h>
#include <livre/core/visitor/VisitState.h>
#include <livre/core/render/Frustum.h>

//#define LIVRE_STATIC_DECOMPOSITION

namespace livre
{
/** Selects all visible rendering nodes */
class SelectVisibles : public DataSourceVisitor
{
public:
    SelectVisibles( const DataSource& dataSource,
                    const Frustum& frustum,
                    const uint32_t windowHeight,
                    const float screenSpaceError,
                    const uint32_t minLOD,
                    const uint32_t maxLOD,
                    const Range& range )
    : DataSourceVisitor( dataSource )
    , _frustum( frustum )
    , _windowHeight( windowHeight )
    , _screenSpaceError( screenSpaceError )
    , _minLOD( minLOD )
    , _maxLOD( maxLOD )
    , _range( range )
    {}

    const NodeIds& getVisibles() const { return _visibles; }

protected:
    void visitPre() final { _visibles.clear(); }

    void visit( const LODNode& lodNode, VisitState& state ) final
    {
        const Boxf& worldBox = lodNode.getWorldBox();
        if( !_frustum.isInFrustum( worldBox ) )
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

       bool lodVisible = isLODVisible( _frustum,
                                       vmin,
                                       worldSpacePerVoxel.find_min(),
                                       _windowHeight,
                                       _screenSpaceError );

       const VolumeInformation& volInfo = getDataSource().getVolumeInfo();
       const uint32_t depth = volInfo.rootNode.getDepth();
       lodVisible = ( lodVisible && lodNode.getRefLevel() >= _minLOD )
                    || ( lodNode.getRefLevel() == _maxLOD )
                    || ( lodNode.getRefLevel() == depth - 1 );

       if( lodVisible )
           _visibles.push_back( lodNode.getNodeId( ));

       state.setVisitChild( !lodVisible );
    }

    bool isLODVisible( const Frustum& frustum,
                      const Vector3f& worldCoord,
                      const float worldSpacePerVoxel,
                      const uint32_t windowHeight,
                      const float screenSpaceError ) const
    {
       const float t = frustum.top();
       const float b = frustum.bottom();

       const float worldSpacePerPixel = ( t - b ) / windowHeight;
       const float pixelPerVoxel = worldSpacePerVoxel / worldSpacePerPixel;

       Vector4f hWorldCoord = worldCoord;
       hWorldCoord[ 3 ] = 1.0f;
       const float distance = std::abs( frustum.getNearPlane().dot( hWorldCoord ));

       const float n = frustum.nearPlane();
       const float pixelPerVoxelInDistance = pixelPerVoxel * n /  ( n + distance );

       return pixelPerVoxelInDistance <= screenSpaceError;
    }

    void visitPost() final
    {
        // Sort-last range selection:
#ifndef LIVRE_STATIC_DECOMPOSITION
        const size_t startIndex = _range[0] * _visibles.size();
        const size_t endIndex = _range[1] * _visibles.size();
#endif
        NodeIds selected;
        for( size_t i = 0; i < _visibles.size(); ++i )
        {
#ifdef LIVRE_STATIC_DECOMPOSITION
            const Range& nodeRange =
                _visibles[ i ].getRange();
            const bool isInRange = nodeRange[ 1 ] > _range[0] &&
                                   nodeRange[ 1 ] <= _range[1];
#else
            const bool isInRange = i >= startIndex && i < endIndex;
#endif
            if( isInRange )
                selected.push_back( _visibles[i] );
        }
        _visibles.swap( selected );
    }

private:

    const Frustum _frustum;
    const uint32_t _windowHeight;
    const float _screenSpaceError;
    const uint32_t _minLOD;
    const uint32_t _maxLOD;
    const Range _range;
    NodeIds _visibles;
};
}
#endif
