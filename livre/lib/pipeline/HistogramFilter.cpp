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
#include <livre/lib/cache/HistogramObject.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/render/Frustum.h>

namespace livre
{
namespace
{
const float infinite = std::numeric_limits< float >::max();
}

struct HistogramFilter::Impl
{
    Impl( Cache& histogramCache,
          const Cache& dataCache,
          const DataSource& dataSource )
        : _histogramCache( histogramCache )
        , _dataCache( dataCache )
        , _dataSource( dataSource )
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
        const auto& frustums = input.get< Frustum >( "Frustum" );
        const auto& viewports = input.get< Viewport >( "RelativeViewport" );

        auto dataSourceRange = input.get< Vector2f >( "DataSourceRange" ).front();
        const auto& frustum = frustums.front();
        const auto& viewport = viewports.front();

        Histogram histogramAccumulated;
        for( const auto& cacheObjects: input.getFutures( "CacheObjects" ))
            for( const auto& cacheObject: cacheObjects.get< ConstCacheObjects >( ))
            {
                const CacheId& cacheId = cacheObject->getId();

                // Hist cache object expands the data source range if data has larger values
                ConstHistogramObjectPtr histCacheObject =
                        _histogramCache.load< HistogramObject >( cacheId,
                                                                 _dataCache,
                                                                 _dataSource,
                                                                 dataSourceRange );
                if( !histCacheObject )
                    continue;

                const Vector2f& currentRange = histCacheObject->getHistogram().getRange();
                if( currentRange[ 0 ] < dataSourceRange[ 0 ] )
                    dataSourceRange[ 0 ] = currentRange[ 0 ];
                histogramAccumulated.setMin(  dataSourceRange[ 0 ] );

                if( currentRange[ 1 ] > dataSourceRange[ 1 ] )
                    dataSourceRange[ 1 ] = currentRange[ 1 ];
                histogramAccumulated.setMax( dataSourceRange[ 1 ] );

                const size_t currentBinCount = histCacheObject->getHistogram().getBins().size();
                if( histogramAccumulated.getBins().empty( ))
                    histogramAccumulated.resize( currentBinCount );

                // When a frame is rendered in multi-channel, multi-node, etc
                // config, some nodes are rendered twice in sort-first renderings
                // To avoid counting nodes twice, we check if the center of the node is in
                // this frustum (because it can only be in one tile at a time). For viewports
                // on the border of the absolute viewport, the frustum is virtually extended
                // to infinity on the boundary.
                try
                {
                    const LODNode& lodNode = _dataSource.getNode( NodeId( cacheId ));
                    if( isCenterInViewport( frustum, lodNode.getWorldBox(), viewport))
                        histogramAccumulated += histCacheObject->getHistogram();
                }
                catch( const std::runtime_error& )
                {
                    // Only compatible histograms can be added.( i.e same data range and number of
                    // bins.) Until data range converges to the full data range combined from other
                    // rendering clients, the cache objects are purged from the cache.
                    _histogramCache.purge( cacheId );
                }
            }

        output.set( "Histogram", histogramAccumulated );
    }

    Cache& _histogramCache;
    const Cache& _dataCache;
    const DataSource& _dataSource;
};

HistogramFilter::HistogramFilter( Cache& histogramCache,
                                  const Cache& dataCache,
                                  const DataSource& dataSource )
    : _impl( new HistogramFilter::Impl( histogramCache,
                                        dataCache,
                                        dataSource ))
{
}

HistogramFilter::~HistogramFilter()
{
}

void HistogramFilter::execute( const FutureMap& input, PromiseMap& output ) const
{
    _impl->execute( input, output );
}
}
