/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <livre/lib/pipeline/HistogramFilter.h>
#include <livre/lib/cache/HistogramCache.h>
#include <livre/lib/cache/TextureDataCache.h>
#include <livre/lib/cache/HistogramObject.h>

#include <livre/core/data/DataSource.h>
#include <livre/core/data/Histogram.h>
#include <livre/core/render/Frustum.h>

namespace livre
{
namespace
{
const float infinite = std::numeric_limits< float >::max();
}

struct HistogramFilter::Impl
{
    Impl( HistogramCache& histogramCache )
        : _histogramCache( histogramCache )
        , _dataSource( histogramCache.getDataCache().getDataSource( ))
        , _components( _dataSource.getVolumeInfo().compCount )
    {}

    bool isCenterInViewport( const Frustum& frustum,
                             const Boxf& worldBox,
                             const Viewport& viewport ) const
    {
        Vector4f center = worldBox.getCenter();
        center[ 3 ] = 1.0;

        Vector4f mvpCenter = frustum.getMVPMatrix() * center;
        Vector3f mvpCenterHom = mvpCenter / mvpCenter[ 3 ];

        const bool isNegXBorder = viewport[ 0 ] == 0.0f; // left
        const bool isPosXBorder = viewport[ 0 ] + viewport[ 2 ] == 1.0f; // left + width
        const bool isNegYBorder = viewport[ 1 ] == 0.0f; // top
        const bool isPosYBorder = viewport[ 1 ] + viewport[ 3 ] == 1.0f; // top + height

        Vector3f minBox( -1.0f );
        Vector3f maxBox( 1.0f );
        if( isNegXBorder )
            minBox[ 0 ] = -infinite;
        if( isPosXBorder )
            maxBox[ 0 ] = infinite;
        if( isNegYBorder )
            minBox[ 1 ] = -infinite;
        if( isPosYBorder )
            maxBox[ 1 ] = infinite;

        minBox[ 2 ] = -infinite;
        maxBox[ 2 ] = infinite;

        const Boxf ndcCube( minBox, maxBox );
        return ndcCube.isIn( mvpCenterHom );
    }

    void execute( const FutureMap& input, PromiseMap& output ) const
    {
        const auto frustum = input.get< Frustum >( "Frustum" ).front();
        const auto viewport = input.get< Viewport >( "RelativeViewport" ).front();

        Histogram histogramAccumulated;
        for( const auto& cacheObjects: input.getFutures( "CacheObjects" ))
            for( const auto& cacheObject: cacheObjects.get< ConstCacheObjects >( ))
            {
                const CacheId& cacheId = cacheObject->getId();
                ConstCacheObjectPtr histCacheObject = _histogramCache.load( cacheId );
                if( !histCacheObject )
                    continue;

                const ConstHistogramObjectPtr& histogramObj =
                        std::static_pointer_cast< const HistogramObject >(
                            histCacheObject );

                // When a frame is rendered in multi-channel, multi-node, etc
                // config, some nodes are rendered twice in sort-first renderings
                // To avoid counting nodes twice, we check if the center of the node is in
                // this frustum (because it can only be in one tile at a time). For viewports
                // on the border of the absolute viewport, the frustum is virtually extended
                // to infinity on the boundary.
                const LODNode& lodNode = _dataSource.getNode( NodeId( cacheId ));
                if( isCenterInViewport( frustum, lodNode.getWorldBox( ), viewport))
                    histogramAccumulated += histogramObj->getHistogram();
            }

        output.set( "Histogram", histogramAccumulated );
    }

    DataInfos getInputDataInfos() const
    {
        return
        {
            { "Frustum" ,getType< Frustum >() },
            { "RelativeViewport" ,getType< Viewport >() },
            { "CacheObjects" ,getType< ConstCacheObjects >() },
        };
    }

    DataInfos getOutputDataInfos() const
    {
        return
        {
            { "Histogram" ,getType< Histogram >() },
        };
    }

    HistogramCache& _histogramCache;
    const DataSource& _dataSource;
    const size_t _components;
};

HistogramFilter::HistogramFilter( HistogramCache& histogramCache )
    : _impl( new HistogramFilter::Impl( histogramCache ))
{
}

HistogramFilter::~HistogramFilter()
{
}

void HistogramFilter::execute( const FutureMap& input, PromiseMap& output ) const
{
    _impl->execute( input, output );
}

DataInfos HistogramFilter::getInputDataInfos() const
{
    return _impl->getInputDataInfos();
}

DataInfos HistogramFilter::getOutputDataInfos() const
{
    return _impl->getOutputDataInfos();
}

}
