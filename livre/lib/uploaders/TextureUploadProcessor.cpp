/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include <livre/lib/cache/TextureCache.h>
#include <livre/lib/cache/TextureDataObject.h>
#include <livre/lib/cache/TextureObject.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>
#include <livre/lib/uploaders/TextureUploadProcessor.h>
#include <livre/lib/visitor/DFSTraversal.h>

#include <livre/core/visitor/RenderNodeVisitor.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/dash/DashRenderNode.h>
#include <livre/core/dash/DashTree.h>
#include <livre/core/render/GLContext.h>
#include <livre/core/render/Renderer.h>

#include <eq/gl.h>

namespace livre
{

#ifdef _ITT_DEBUG_
#include "ittnotify.h"
__itt_domain* ittTextureLoadDomain = __itt_domain_create("Texture Loading");
__itt_string_handle* ittTextureComputationTask =
        __itt_string_handle_create("Texture loading computation");
__itt_string_handle* ittTextureLoadTask = __itt_string_handle_create("Texture loading task");
#endif // _ITT_DEBUG_


// Dont forget to apply cache policy after each traversal
class TextureLoaderVisitor : public RenderNodeVisitor
{
public:
    TextureLoaderVisitor( DashTree& dashTree,
                          TextureCache& textureCache,
                          ProcessorInputPtr processorInput,
                          ProcessorOutputPtr processorOutput,
                          bool& needRedraw )
        : RenderNodeVisitor( dashTree )
        , _cache( textureCache )
        , _input( processorInput )
        , _output( processorOutput )
        , _allLoaded( true ) // be optimistic; will be set to false on first
                             // non-loaded data during visit
        , _synchronous( false )
        , _needRedraw( needRedraw )
    {}

    void visit( DashRenderNode& renderNode, VisitState& state ) final;

    bool isAllDataLoaded() const { return _allLoaded; }

    bool isSynchronous() const { return _synchronous; }
    void setSynchronous( const bool synchronous ) { _synchronous = synchronous;}

private:
    TextureCache& _cache;
    ProcessorInputPtr _input;
    ProcessorOutputPtr _output;
    bool _allLoaded;
    bool _synchronous;
    bool& _needRedraw;
};

class CollectVisiblesVisitor : public RenderNodeVisitor
{
public:
    CollectVisiblesVisitor( DashTree& dashTree )
     : RenderNodeVisitor( dashTree ) {}

    void visit( DashRenderNode& renderNode, VisitState& state ) final
    {
        if( !renderNode.isInFrustum( ) || renderNode.isLODVisible( ))
             state.setVisitChild( false );
    }
};

TextureUploadProcessor::TextureUploadProcessor( DashTree& dashTree,
                                                GLContextPtr shareContext,
                                                TextureDataCache& dataCache,
                                                const VolumeRendererParameters& vrParameters )
    : _dashTree( dashTree )
    , _glContext( shareContext->clone( ))
    , _sharedContext( shareContext )
    , _currentFrameID( 0 )
    , _threadOp( TO_NONE )
    , _vrParameters( vrParameters )
    , _textureCache( dataCache,
                     _vrParameters.getMaxGPUCacheMemoryMB() * LB_1MB,
                     GL_LUMINANCE8 )
    , _allDataLoaded( false )
    , _needRedraw( false )
{
    setDashContext( dashTree.createContext());
}

TextureUploadProcessor::~TextureUploadProcessor()
{
}

const TextureCache& TextureUploadProcessor::getTextureCache() const
{
    return _textureCache;
}

bool TextureUploadProcessor::initializeThreadRun_()
{
    setName( "TexUp" );
    LBASSERT( _glContext );
    return DashProcessor::initializeThreadRun_();
}

bool TextureUploadProcessor::onPreCommit_( const uint32_t outputConnection LB_UNUSED )
{
    const bool ret = _allDataLoaded;

    _allDataLoaded = false;
    return ret;
}

void TextureUploadProcessor::_loadData()
{
    TextureLoaderVisitor loadVisitor( _dashTree, _textureCache,
                                      processorInputPtr_, processorOutputPtr_,
                                      _needRedraw );

    loadVisitor.setSynchronous( _vrParameters.getSynchronousMode( ));

    DFSTraversal traverser;
    const RootNode& rootNode = _dashTree.getDataSource()->getVolumeInfo().rootNode;
    traverser.traverse( rootNode, loadVisitor, _currentFrameID );

    if(  _vrParameters.getSynchronousMode( ))
        _allDataLoaded = loadVisitor.isAllDataLoaded();
    else
        _allDataLoaded = true;
}

void TextureUploadProcessor::runLoop_()
{
    _needRedraw = false;
    if( GLContext::getCurrent() != _glContext.get( ))
    {
        _glContext->share( *_sharedContext );
        _glContext->makeCurrent();
    }

    processorInputPtr_->applyAll( CONNECTION_ID );
    _checkThreadOperation();

#ifdef _ITT_DEBUG_
    __itt_task_begin ( ittTextureLoadDomain, __itt_null, __itt_null, ittTextureComputationTask );
#endif //_ITT_DEBUG_

    const DashRenderStatus& renderStatus = _dashTree.getRenderStatus();
    if( renderStatus.getFrameID() != _currentFrameID )
    {
        _protectUnloading.clear();
        CollectVisiblesVisitor collectVisibles( _dashTree );
        DFSTraversal traverser;
        const RootNode& rootNode =
                _dashTree.getDataSource()->getVolumeInfo().rootNode;
        traverser.traverse( rootNode, collectVisibles, renderStatus.getFrameID( ));
        _currentFrameID = renderStatus.getFrameID();
    }
    _loadData();
    processorOutputPtr_->commit( CONNECTION_ID );

#ifdef _ITT_DEBUG_
    __itt_task_end( ittTextureLoadDomain );
#endif //_ITT_DEBUG_
}

void TextureUploadProcessor::_checkThreadOperation()
{
    DashRenderStatus& renderStatus = _dashTree.getRenderStatus();
    ThreadOperation op = renderStatus.getThreadOp();
    if( op != _threadOp )
    {
        _threadOp = op;
        renderStatus.setThreadOp( op );
        processorOutputPtr_->commit( CONNECTION_ID );
    }

    if( _threadOp == TO_EXIT )
        exit();
}

void TextureLoaderVisitor::visit( DashRenderNode& renderNode, VisitState& state )
{
    const LODNode& lodNode = renderNode.getLODNode();
    if( !lodNode.isValid( ))
        return;

    if( !renderNode.isInFrustum( ))
    {
        state.setVisitChild( false );
        return;
    }

    if( !renderNode.isLODVisible( ))
        return;

    state.setVisitChild( false );

    const ConstCacheObjectPtr texPtr = renderNode.getTextureObject();
    if( texPtr && texPtr->isLoaded( ))
        return;

    ConstCacheObjectPtr texture = _cache.get( lodNode.getNodeId().getId( ));
    if( texture && texture->isLoaded() )
    {
        renderNode.setTextureObject( texture );
        _output->commit( CONNECTION_ID );
        return;
    }
    else
    {
        const ConstCacheObjectPtr textureData = renderNode.getTextureDataObject();
        if( textureData && textureData->isLoaded( ))
        {
#ifdef _ITT_DEBUG_
            __itt_task_begin ( ittTextureLoadDomain, __itt_null, __itt_null,
                               ittTextureLoadTask );
#endif //_ITT_DEBUG_
            CacheObjectPtr lodTexture = _cache.load( lodNode.getNodeId().getId( ));

#ifdef _ITT_DEBUG_
            __itt_task_end( ittTextureLoadDomain );
#endif //_ITT_DEBUG_
            renderNode.setTextureObject( lodTexture );
            renderNode.setTextureDataObject( textureData );
            _output->commit( CONNECTION_ID );
            _needRedraw = true;
        }
        else
        {
            _allLoaded = false;
            LBVERB << "Texture data not loaded:" << lodNode.getNodeId() << std::endl;
        }
    }

    if( !isSynchronous( ))
        // only in asynchronous mode
        state.setBreakTraversal( _input->dataWaitingOnInput( CONNECTION_ID ));
}

}
