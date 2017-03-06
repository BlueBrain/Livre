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

#include <livre/lib/cache/DataObject.h>
#include <livre/lib/cache/TextureObject.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>
#include <livre/lib/pipeline/DataUploadFilter.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/data/NodeId.h>
#include <livre/core/pipeline/Pipeline.h>

#include <eq/gl.h>

namespace livre
{
struct DataUploadFilter::Impl
{
public:
    Impl(const size_t id, const size_t nUploaders, Cache& dataCache,
         Cache& textureCache, DataSource& dataSource, TexturePool& texturePool)
        : _id(id)
        , _nUploaders(nUploaders)
        , _dataCache(dataCache)
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
        const size_t perThreadSize =
            std::max((size_t)1, visibles.size() / _nUploaders);

        NodeIds partialVisibles;

        if (_id * perThreadSize < visibles.size())
        {
            const NodeId* begin = visibles.data() + perThreadSize * _id;
            if (_id == _nUploaders - 1) // last item
                partialVisibles =
                    NodeIds(begin,
                            begin + (visibles.size() - (perThreadSize * _id)));
            else
                partialVisibles = NodeIds(begin, begin + perThreadSize);
        }

        if (isAsync)
        {
            output.set("CacheObjects",
                       get(partialVisibles)); // Already loaded ones
            load(partialVisibles);            // load
        }
        else
            output.set("CacheObjects", load(partialVisibles)); // load all
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

    const size_t _id;
    const size_t _nUploaders;
    Cache& _dataCache;
    Cache& _textureCache;
    DataSource& _dataSource;
    TexturePool& _texturePool;
};

DataUploadFilter::DataUploadFilter(const size_t id, const size_t nUploaders,
                                   Cache& dataCache, Cache& textureCache,
                                   DataSource& dataSource,
                                   TexturePool& texturePool)
    : _impl(new DataUploadFilter::Impl(id, nUploaders, dataCache, textureCache,
                                       dataSource, texturePool))
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
