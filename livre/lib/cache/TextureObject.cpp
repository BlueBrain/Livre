/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/core/cache/Cache.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/render/GLContext.h>
#include <livre/core/render/Renderer.h>
#include <livre/core/render/TexturePool.h>

#include <eq/gl.h>

namespace livre
{
namespace
{
size_t getTextureSize(const DataSource& dataSource)
{
    const Vector3ui& textureSize = dataSource.getVolumeInfo().maximumBlockSize;
    return textureSize.product() *
           dataSource.getVolumeInfo().getBytesPerVoxel();
}
}

#define glewGetContext() GLContext::glewGetContext()

/**
 * The TextureObject class holds the informarmation for the data which is on the
 * GPU.
  */
struct TextureObject::Impl
{
    Impl(const CacheId& cacheId, const Cache& dataCache,
         const DataSource& dataSource, TexturePool& texturePool)
        : _textureState(texturePool)
        , _textureSize(getTextureSize(dataSource))
    {
        LBASSERT(_textureState.textureId);
        if (!load(cacheId, dataCache, dataSource, texturePool))
            LBTHROW(
                CacheLoadException(cacheId,
                                   "Unable to construct texture cache object"));
    }

    ~Impl() {}
    bool load(const CacheId& cacheId, const Cache& dataCache,
              const DataSource& dataSource, const TexturePool& texturePool)
    {
        ConstDataObjectPtr data = dataCache.get<DataObject>(cacheId);
        if (!data)
            return false;

        initialize(cacheId, dataSource, texturePool, data);
        return true;
    }

    void initialize(const CacheId& cacheId, const DataSource& dataSource,
                    const TexturePool& texturePool,
                    const ConstDataObjectPtr& data)
    {
        // TODO: The internal format size should be calculated correctly
        const Vector3f& overlap = dataSource.getVolumeInfo().overlap;
        const LODNode& lodNode = dataSource.getNode(NodeId(cacheId));
        const Vector3f& size = lodNode.getVoxelBox().getSize();
        const Vector3f& maxSize = dataSource.getVolumeInfo().maximumBlockSize;
        const Vector3f& overlapf = overlap / maxSize;
        _textureState.textureCoordsMax = overlapf + size / maxSize;
        _textureState.textureCoordsMin = overlapf;
        _textureState.textureSize =
            _textureState.textureCoordsMax - _textureState.textureCoordsMin;

        loadTextureToGPU(lodNode, dataSource, texturePool, data);
    }

    bool loadTextureToGPU(const LODNode& lodNode, const DataSource& dataSource,
                          const TexturePool& texturePool,
                          const ConstDataObjectPtr& data) const
    {
#ifdef LIVRE_DEBUG_RENDERING
        std::cout << "Upload " << lodNode.getNodeId().getLevel() << ' '
                  << lodNode.getRelativePosition() << " to "
                  << _textureState.textureId << std::endl;
#endif
        const Vector3ui& overlap = dataSource.getVolumeInfo().overlap;
        const Vector3ui& voxSizeVec = lodNode.getBlockSize() + overlap * 2;
        _textureState.bind();

        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, voxSizeVec[0], voxSizeVec[1],
                        voxSizeVec[2], texturePool.getFormat(),
                        texturePool.getTextureType(), data->getDataPtr());

        const GLenum glErr = glGetError();
        if (glErr != GL_NO_ERROR)
        {
            LBERROR << "Error loading the texture into GPU, error number : "
                    << glErr << std::endl;
            return false;
        }

        return true;
    }

    TextureState _textureState;
    size_t _textureSize;
};

TextureObject::TextureObject(const CacheId& cacheId, const Cache& dataCache,
                             const DataSource& dataSource,
                             TexturePool& texturePool)
    : CacheObject(cacheId)
    , _impl(new Impl(cacheId, dataCache, dataSource, texturePool))
{
}

TextureObject::~TextureObject()
{
}

const TextureState& TextureObject::getTextureState() const
{
    return _impl->_textureState;
}

size_t TextureObject::getSize() const
{
    return _impl->_textureSize;
}
}
