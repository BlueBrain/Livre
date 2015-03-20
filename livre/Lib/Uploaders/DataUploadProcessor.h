
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

#ifndef _DataLoadProcessor_h_
#define _DataLoadProcessor_h_

#include <lunchbox/clock.h>

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
class DataUploadProcessor : public DashProcessor, public GLContextTrait
{
public:

    /**
     * @param dashTree The dash node hierarchy.
     * @param rawDataCache Raw data cache that holds the raw data in the CPU memory.
     * @param textureDataCache Texture data cache holds the data in the CPU memory.
     */
    DataUploadProcessor( DashTreePtr dashTree,
                         RawDataCache& rawDataCache,
                         TextureDataCache& textureDataCache );

    /**
     * @param glWidgetPtr Set the GL window to update.
     */
    void setGLWidget( GLWidgetPtr glWidgetPtr );

private:

    bool initializeThreadRun_( ) final;
    void runLoop_( ) final;
    void _loadData();
    bool _loadPrioritizedData( const Frustum& frustum,
                               const LoadPriority priority );

    DashTreePtr _dashTree;
    RawDataCache& _rawDataCache;
    TextureDataCache& _textureDataCache;
    uint64_t _currentFrameID;
    void _checkThreadOperation( );
    ThreadOperation _threadOp;
    GLWidgetPtr _glWidgetPtr;
};

}

#endif // _DataLoadProcessor_h_
