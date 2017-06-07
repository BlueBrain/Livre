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

#include <livre/lib/cache/TextureObject.h>
#include <livre/lib/pipeline/RenderFilter.h>

#include <livre/core/render/Renderer.h>
#include <livre/data/DataSource.h>
#include <livre/data/Frustum.h>

namespace livre
{
struct RenderFilter::Impl
{
    Impl(const DataSource& dataSource, Renderer& renderer)
        : _dataSource(dataSource)
        , _renderer(renderer)
    {
    }

    void execute(const FutureMap& input, PromiseMap&) const
    {
        NodeIds renderBricks;
        for (const auto& cacheObjects : input.getFutures("CacheObjects"))
        {
            const auto& objects = cacheObjects.get<ConstCacheObjects>();
            renderBricks.reserve(renderBricks.size() + objects.size());
            for (const auto& cacheObject : objects)
            {
                const ConstTextureObjectPtr& texture =
                    std::static_pointer_cast<const TextureObject>(cacheObject);

                renderBricks.emplace_back(texture->getId());
            }
        }

        const auto& frustums = input.get<Frustum>("Frustum");
        const auto& clipPlanes = input.get<ClipPlanes>("ClipPlanes");
        const auto& viewports = input.get<PixelViewport>("Viewport");
        const auto& renderStages = input.get<uint32_t>("RenderStages");
        _renderer.render(frustums[0], clipPlanes[0], viewports[0], renderBricks,
                         renderStages[0]);
    }

    DataInfos getInputDataInfos() const
    {
        return {{"CacheObjects", getType<ConstCacheObjects>()},
                {"Frustum", getType<Frustum>()},
                {"Viewport", getType<PixelViewport>()},
                {"ClipPlanes", getType<ClipPlanes>()},
                {"RenderStages", getType<uint32_t>()}};
    }

    const DataSource& _dataSource;
    Renderer& _renderer;
};

RenderFilter::RenderFilter(const DataSource& dataSource, Renderer& renderer)
    : _impl(new RenderFilter::Impl(dataSource, renderer))
{
}

RenderFilter::~RenderFilter()
{
}

void RenderFilter::execute(const FutureMap& input, PromiseMap& output) const
{
    _impl->execute(input, output);
}

DataInfos RenderFilter::getInputDataInfos() const
{
    return _impl->getInputDataInfos();
}
}
