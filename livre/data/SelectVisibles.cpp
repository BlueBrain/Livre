/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
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

#include "SelectVisibles.h"

#include <livre/data/LODNode.h>
#include <livre/data/types.h>
//#define LIVRE_STATIC_DECOMPOSITION

namespace livre
{
struct SelectVisibles::Impl
{
    Impl(const DataSource& dataSource, const Frustum& frustum,
         const uint32_t windowHeight, const float screenSpaceError,
         const uint32_t minLOD, const uint32_t maxLOD, const Range& range,
         const ClipPlanes& clipPlanes)
        : _dataSource(dataSource)
        , _frustum(frustum)
        , _windowHeight(windowHeight)
        , _screenSpaceError(screenSpaceError)
        , _minLOD(minLOD)
        , _maxLOD(maxLOD)
        , _range(range)
        , _clipPlanes(clipPlanes)
    {
    }

    bool isLODVisible(const Vector3f& worldCoord,
                      const float worldSpacePerVoxel) const
    {
        const float t = _frustum.top();
        const float b = _frustum.bottom();

        const float worldSpacePerPixel = (t - b) / _windowHeight;
        const float pixelPerVoxel = worldSpacePerVoxel / worldSpacePerPixel;

        Vector4f hWorldCoord = worldCoord;
        hWorldCoord[3] = 1.0f;
        const float distance =
            std::abs(_frustum.getNearPlane().dot(hWorldCoord));

        const float n = _frustum.nearPlane();
        const float pixelPerVoxelInDistance =
            pixelPerVoxel * n / (n + distance);

        return pixelPerVoxelInDistance <= _screenSpaceError;
    }

    bool visit(const LODNode& lodNode)
    {
        const Boxf& worldBox = lodNode.getWorldBox();
        if (!_frustum.isInFrustum(worldBox) || _clipPlanes.isOutside(worldBox))
            return false;

        Vector3f vmin, vmax;
        const Plane& nearPlane = _frustum.getNearPlane();

        worldBox.computeNearFar(nearPlane, vmin, vmax);

        Vector4f hVmin = vmin;
        hVmin[3] = 1.0f;

        Vector4f hVmax = vmax;
        hVmax[3] = 1.0f;

        // The bounding box intersects the plane
        if (_frustum.getNearPlane().dot(hVmin) < 0 ||
            _frustum.getNearPlane().dot(hVmax) < 0)
        {
            // Where eye direction intersects with near plane
            vmin = _frustum.getEyePos() -
                   _frustum.getViewDir() * _frustum.nearPlane();
        }

        const Vector3f& voxelBox = lodNode.getVoxelBox().getSize();
        const Vector3f& worldSpacePerVoxel = worldBox.getSize() / voxelBox;

        bool lodVisible = isLODVisible(vmin, worldSpacePerVoxel.find_min());

        const VolumeInformation& volInfo = _dataSource.getVolumeInfo();
        const uint32_t depth = volInfo.rootNode.getDepth();
        lodVisible = (lodVisible && lodNode.getRefLevel() >= _minLOD) ||
                     (lodNode.getRefLevel() == _maxLOD) ||
                     (lodNode.getRefLevel() == depth - 1);

        if (lodVisible)
            _visibles.push_back(lodNode.getNodeId());

        return !lodVisible;
    }

    void visitPre() { _visibles.clear(); }
    void visitPost()
    {
// Sort-last range selection:
#ifndef LIVRE_STATIC_DECOMPOSITION
        const size_t startIndex = _range[0] * _visibles.size();
        const size_t endIndex = _range[1] * _visibles.size();
#endif
        NodeIds selected;
        for (size_t i = 0; i < _visibles.size(); ++i)
        {
#ifdef LIVRE_STATIC_DECOMPOSITION
            const Range& nodeRange = _visibles[i].getRange();
            const bool isInRange =
                nodeRange[1] > _range[0] && nodeRange[1] <= _range[1];
#else
            const bool isInRange = i >= startIndex && i < endIndex;
#endif
            if (isInRange)
                selected.push_back(_visibles[i]);
        }
        _visibles.swap(selected);
    }

    const DataSource& _dataSource;
    const Frustum _frustum;
    const uint32_t _windowHeight;
    const float _screenSpaceError;
    const uint32_t _minLOD;
    const uint32_t _maxLOD;
    const Range _range;
    NodeIds _visibles;
    const ClipPlanes _clipPlanes;
};

SelectVisibles::SelectVisibles(const DataSource& dataSource,
                               const Frustum& frustum,
                               const uint32_t windowHeight,
                               const float screenSpaceError,
                               const uint32_t minLOD, const uint32_t maxLOD,
                               const Range& range, const ClipPlanes& clipPlanes)
    : DataSourceVisitor(dataSource)
    , _impl(new SelectVisibles::Impl(dataSource, frustum, windowHeight,
                                     screenSpaceError, minLOD, maxLOD, range,
                                     clipPlanes))
{
}

SelectVisibles::~SelectVisibles()
{
}

const NodeIds& SelectVisibles::getVisibles() const
{
    return _impl->_visibles;
}

void SelectVisibles::visitPre()
{
    _impl->visitPre();
}

bool SelectVisibles::visit(const LODNode& lodNode)
{
    return _impl->visit(lodNode);
}

void SelectVisibles::visitPost()
{
    _impl->visitPost();
}
}
