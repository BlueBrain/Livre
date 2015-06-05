
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

#include <livre/Lib/types.h>

#include <livre/core/DashPipeline/DashProcessor.h>
#include <livre/core/Dash/DashRenderStatus.h>
#include <livre/core/Render/GLContextTrait.h>
#include <livre/Lib/Cache/LRUCachePolicy.h>
#include <livre/Lib/Cache/TextureCache.h>

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
     * @param maxTextureMemory the maximum memory in MB for texture cache
     */
    TextureUploadProcessor( DashTreePtr dashTree,
                            GLContextPtr shareContext,
                            GLContextPtr context,
                            const uint32_t maxTextureMemory );

private:
    bool onPreCommit_( const uint32_t connection ) final;
    void onPostCommit_( const uint32_t connection, const CommitState state ) final;
    bool initializeThreadRun_( ) final;
    void runLoop_( ) final;

    void _loadData();
    void _checkThreadOperation( );

    DashTreePtr _dashTree;
    GLContextPtr _shareContext;
    TextureCache _textureCache;
    LRUCachePolicy _cachePolicy;
    uint64_t _currentFrameID;
    ThreadOperation _threadOp;
    CacheIdSet _protectUnloading;
    bool _firstTimeLoaded;
};

}

#endif // _TextureLoadProcessor_h_
