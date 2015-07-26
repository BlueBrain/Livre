/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet.Bilgili@epfl.ch
 *                          Daniel.Nachbaur@epfl.ch
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

#ifndef _DataLoadProcessor_h_
#define _DataLoadProcessor_h_

#include <lunchbox/clock.h>

#include <livre/Lib/api.h>
#include <livre/Lib/Visitor/DFSTraversal.h>
#include <livre/Lib/types.h>

#include <livre/core/DashPipeline/DashProcessor.h>
#include <livre/core/Visitor/NodeVisitor.h>
#include <livre/core/Dash/DashRenderStatus.h>
#include <livre/core/Render/GLContextTrait.h>

namespace livre
{

/**
 * The DataUploadProcessor class is responsible for loading data into CPU memory.
 * It is derived from GLContextTrait class because some algorithms may need OpenGL
 * to generate the data.
 */
class DataUploadProcessor : public DashProcessor, private GLContextTrait
{
public:
    /**
     * @param dashTree The dash node hierarchy.
     * @param shareContext the context which this processors shares against.
     * @param context the context used by this processor.
     * @param textureDataCache Texture data cache holds the data in the CPU memory.
     */
    LIVRE_API DataUploadProcessor( DashTreePtr dashTree,
                                   GLContextPtr shareContext,
                                   GLContextPtr context,
                                   TextureDataCache& textureDataCache );
private:
    bool initializeThreadRun_( ) final;
    void runLoop_( ) final;
    void _loadData();

    DashTreePtr _dashTree;
    GLContextPtr _shareContext;
    TextureDataCache& _textureDataCache;
    uint64_t _currentFrameID;
    void _checkThreadOperation( );
    ThreadOperation _threadOp;
};

}

#endif // _DataLoadProcessor_h_
