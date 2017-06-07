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

#include <livre/lib/cache/DataObject.h>
#include <livre/lib/cache/TextureObject.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>
#include <livre/lib/pipeline/DataUploadFilter.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/pipeline/Pipeline.h>
#include <livre/data/NodeId.h>

#include <eq/gl.h>

namespace livre
{
struct DataUploadFilter::Impl
{
public:
    Impl(Cache& dataCache, Cache& textureCache, DataSource& dataSource,
         TexturePool& texturePool)
        : _dataCache(dataCache)
        , _textureCache(textureCache)
        , _dataSource(dataSource)
        , _texturePool(texturePool)
    {
    }

    ConstCacheObjects load(const NodeIds& visibles) const
    {
        ConstCacheObjects cacheObjects;
        cacheObjects.reserve(visibles.size());
        bool isTextureUploaded = false;
        for (const NodeId& nodeId : visibles)
        {
            ConstTextureObjectPtr texture =
                _textureCache.get<TextureObject>(nodeId.getId());
            if (!texture)
            {
                if (!_dataCache.load<DataObject>(nodeId.getId(), _dataSource))
                    continue;

                texture =
                    _textureCache.load<TextureObject>(nodeId.getId(),
                                                      _dataCache, _dataSource,
                                                      _texturePool);
                if (!texture)
                    continue;

                cacheObjects.push_back(texture);
                isTextureUploaded = true;
            }
            else
                cacheObjects.push_back(texture);
        }

        if (isTextureUploaded)
            glFinish();

        return cacheObjects;
    }

    ConstCacheObjects get(const NodeIds& visibles) const
    {
        ConstCacheObjects cacheObjects;
        cacheObjects.reserve(visibles.size());
        for (const NodeId& nodeId : visibles)
        {
            ConstCacheObjectPtr texture =
                _textureCache.get<TextureObject>(nodeId.getId());
            if (texture)
                cacheObjects.push_back(texture);
        }

        return cacheObjects;
    }

    void execute(const FutureMap& input, PromiseMap& output) const
    {
        const UniqueFutureMap uniqueInputs(input.getFutures());
        const auto& vrParams =
            uniqueInputs.get<VolumeRendererParameters>("Params");

        const auto& visibles = uniqueInputs.get<NodeIds>("VisibleNodes");

        const bool isAsync = !vrParams.getSynchronousMode();

        if (isAsync)
        {
            output.set("CacheObjects", get(visibles)); // Already loaded ones

            // only load 1 missing texture at a time aka per frame. This might
            // seem a waste of bandwidth but leads to a more responsive
            // application as blocks which are not visible anymore won't still
            // be queued for uploading.
            for (const auto& node : visibles)
            {
                if (!_textureCache.get<TextureObject>(node.getId()))
                {
                    load(NodeIds(1, node)); // load
                    break;
                }
            }
        }
        else
            output.set("CacheObjects", load(visibles)); // load all
    }

    DataInfos getInputDataInfos() const
    {
        return {
            {"Params", getType<VolumeRendererParameters>()},
            {"VisibleNodes", getType<NodeIds>()},
        };
    }

    DataInfos getOutputDataInfos() const
    {
        return {
            {"CacheObjects", getType<ConstCacheObjects>()},
        };
    }

    Cache& _dataCache;
    Cache& _textureCache;
    DataSource& _dataSource;
    TexturePool& _texturePool;
};

DataUploadFilter::DataUploadFilter(Cache& dataCache, Cache& textureCache,
                                   DataSource& dataSource,
                                   TexturePool& texturePool)
    : _impl(new DataUploadFilter::Impl(dataCache, textureCache, dataSource,
                                       texturePool))
{
}

DataUploadFilter::~DataUploadFilter()
{
}

void DataUploadFilter::execute(const FutureMap& input, PromiseMap& output) const
{
    _impl->execute(input, output);
}

DataInfos DataUploadFilter::getInputDataInfos() const
{
    return _impl->getInputDataInfos();
}

DataInfos DataUploadFilter::getOutputDataInfos() const
{
    return _impl->getOutputDataInfos();
}
}
