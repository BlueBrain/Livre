
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

#include <lunchbox/clock.h>

#include <livre/core/DashPipeline/DashProcessor.h>
#include <livre/core/Visitor/NodeVisitor.h>
#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Render/GLContextTrait.h>
#include <livre/Lib/Cache/LRUCachePolicy.h>

namespace livre
{

/**
 * @brief The TextureLoadProcessor class is responsible for loading texture data to GPU.
 */
class TextureUploadProcessor : public DashProcessor, public GLContextTrait
{
public:

    /**
     * @brief TextureLoadProcessor constructor.
     * @param textureCache Texture cache.
     */
    TextureUploadProcessor( TextureCache &textureCache );

    /**
     * @brief setGLWidget sets the GL window to update
     * @param glWidgetPtr GLWidget to send update commands.
     */
    void setGLWidget( GLWidgetPtr glWidgetPtr );

    /**
     * @brief setDashTree Sets the dash tree.
     * @param dashTree Hieararchical Dash Tree respresenting the volume.
     */
    void setDashTree( dash::NodePtr dashTree );

private:

    virtual bool onPreCommit_( const uint32_t connection );

    virtual void onPostCommit_( const uint32_t connection, const CommitState state );

    virtual bool initializeThreadRun_( );

    virtual void runLoop_( );

    void loadData_();

    void checkThreadOperation_( );

    dash::NodePtr dashTree_;

    GLWidgetPtr glWidgetPtr_;

    TextureCache& textureCache_;

    LRUCachePolicy cachePolicy_;

    uint64_t currentFrameID_;

    ThreadOperation threadOp_;

    CacheIdSet protectUnloading_;

    bool firstTimeLoaded_;
};

}

#endif // _TextureLoadProcessor_h_
