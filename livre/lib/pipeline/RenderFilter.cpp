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

#include <livre/lib/pipeline/RenderFilter.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/render/Renderer.h>
#include <livre/core/render/RenderInputs.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/ClipPlanes.h>
#include <livre/core/data/DataSource.h>

namespace livre
{

struct RenderFilter::Impl
{
    Impl( const DataSource& dataSource,
          Renderer& renderer )
        : _dataSource( dataSource )
        , _renderer( renderer )
    {}

    void execute( const FutureMap& input, PromiseMap&) const
    {
        ConstCacheObjects renderBricks;
        for( const auto& cacheObjects: input.getFutures( "CacheObjects" ))
             for( const auto& cacheObject: cacheObjects.get< ConstCacheObjects >( ))
                renderBricks.push_back( cacheObject );

        const auto renderInputs = input.get< RenderInputs >( "RenderInputs" )[ 0 ];
        const auto renderStages = input.get< uint32_t >( "RenderStages" )[ 0 ];

        _renderer.render( renderInputs, renderBricks, renderStages );
    }

    const DataSource& _dataSource;
    Renderer& _renderer;
};

RenderFilter::RenderFilter( const DataSource& dataSource,
                            Renderer& renderer )
    : _impl( new RenderFilter::Impl( dataSource, renderer ))
{}

RenderFilter::~RenderFilter()
{}

void RenderFilter::execute( const FutureMap& input,
                            PromiseMap& output ) const
{
    _impl->execute( input, output );
}
}
