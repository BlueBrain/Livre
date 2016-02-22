
/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                          Ahmet.Bilgili@epfl.ch
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

#ifndef _TextureLoadProcessor_h_
#define _TextureLoadProcessor_h_

#include <livre/lib/api.h>
#include <livre/lib/types.h>

#include <livre/core/dashpipeline/DashProcessor.h>
#include <livre/core/dash/DashRenderStatus.h>
#include <livre/core/render/GLContextTrait.h>
#include <livre/lib/cache/TextureCache.h>

namespace livre
{

/**
 * The TextureLoadProcessor class is responsible for loading texture data to GPU.
 */
class TextureUploadProcessor : public DashProcessor, private GLContextTrait
{
public:
    /**
     * @param dashTree The dash node hierarchy.
     * @param shareContext the context which this processors shares against.
     * @param context the context used by this processor.
     * @param vrParameters the volume rendering parameters.
     */
    LIVRE_API TextureUploadProcessor( DashTree& dashTree,
                                      GLContextPtr shareContext,
                                      GLContextPtr context,
                                      TextureDataCache& dataCache,
                                      const VolumeRendererParameters& vrParameters );

    LIVRE_API ~TextureUploadProcessor();

    /** @return the texture cache */
    LIVRE_API const TextureCache& getTextureCache() const;

protected:
    LIVRE_API bool onPreCommit_( uint32_t connection ) override;
    bool needRedraw() const { return _needRedraw; }

private:
    LIVRE_API bool initializeThreadRun_( ) final;
    LIVRE_API void runLoop_( ) final;

    void _loadData();
    void _checkThreadOperation( );

    DashTree& _dashTree;
    GLContextPtr _shareContext;

    uint64_t _currentFrameID;
    ThreadOperation _threadOp;
    CacheIdSet _protectUnloading;
    const VolumeRendererParameters& _vrParameters;
    TextureCache _textureCache;
    bool _allDataLoaded;
    bool _needRedraw;
};

}

#endif // _TextureLoadProcessor_h_
