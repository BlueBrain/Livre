/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
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

#include <livre/Lib/Cache/LRUCachePolicy.h>
#include <livre/Lib/Cache/TextureCache.h>
#include <livre/Lib/Cache/TextureDataObject.h>
#include <livre/Lib/Cache/TextureObject.h>
#include <livre/Lib/Configuration/VolumeRendererParameters.h>
#include <livre/Lib/Uploaders/TextureUploadProcessor.h>
#include <livre/Lib/Visitor/DFSTraversal.h>

#include <livre/core/Visitor/RenderNodeVisitor.h>
#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Dash/DashTree.h>
#include <livre/core/Render/GLContext.h>
#include <livre/core/Render/GLWidget.h>
#include <livre/core/Render/Renderer.h>
#include <livre/core/Render/gl.h>

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

    enum LoadPriority
    {
        LP_VISIBLE,
        LP_TEXTURE
    };

    TextureLoaderVisitor( DashTreePtr dashTree,
                          TextureCache& textureCache,
                          ProcessorInputPtr processorInput,
                          ProcessorOutputPtr processorOutput,
                          LoadPriority loadPriority  )
        : RenderNodeVisitor( dashTree ),
          textureCache_( textureCache ),
          processorInput_( processorInput ),
          processorOutput_( processorOutput ),
          loadPriority_( loadPriority )
    {}

    void visit( DashRenderNode& renderNode, VisitState& state ) final;
    void setLoadPriority( LoadPriority loadPriority ) { loadPriority_ = loadPriority; }

private:

    TextureCache& textureCache_;
    ProcessorInputPtr processorInput_;
    ProcessorOutputPtr processorOutput_;
    LoadPriority loadPriority_;
};

class CollectVisiblesVisitor : public RenderNodeVisitor
{
public:
    CollectVisiblesVisitor( DashTreePtr dashTree,
                            CacheIdSet& currentVisibleSet )
     : RenderNodeVisitor( dashTree ),
       currentVisibleSet_( currentVisibleSet ) {}

    void visit( DashRenderNode& renderNode, VisitState& state ) final
    {
        const LODNode& lodNode = renderNode.getLODNode();

        if( !lodNode.isValid() ||
             lodNode.getRefLevel() == 0 ||
             !renderNode.isVisible() )
        {
            return;
        }

        currentVisibleSet_.insert( lodNode.getNodeId().getId( ));
        state.setVisitChild( false );
    }

private:

    CacheIdSet& currentVisibleSet_;
};

TextureUploadProcessor::TextureUploadProcessor( DashTreePtr dashTree,
                                                TextureCache &textureCache )
    : _dashTree( dashTree ),
      _textureCache( textureCache ),
      _currentFrameID( 0 ),
      _threadOp( TO_NONE ),
      _firstTimeLoaded( false )
{
    setDashContext( dashTree->createContext());
}

void TextureUploadProcessor::setGLWidget( GLWidgetPtr glWidgetPtr )
{
    _glWidgetPtr = glWidgetPtr;
}

bool TextureUploadProcessor::initializeThreadRun_()
{
    setName( "TextureUpload" );
    if( !DashProcessor::initializeThreadRun_( ) )
        return false;

    return true;
}

bool TextureUploadProcessor::onPreCommit_( const uint32_t outputConnection LB_UNUSED )
{
    if( !_firstTimeLoaded )
        return false;

    return true;
}

void TextureUploadProcessor::onPostCommit_( const uint32_t connection LB_UNUSED,
                                            const CommitState state LB_UNUSED )
{
    if( _glWidgetPtr && state != CS_NOCHANGE )
        glFinish( );

    if( _glWidgetPtr && state == CS_COMMITED )
        _glWidgetPtr->update();
}

void TextureUploadProcessor::_loadData()
{
    TextureLoaderVisitor loadVisitor( _dashTree,
                                      _textureCache,
                                      processorInputPtr_,
                                      processorOutputPtr_,
                                      TextureLoaderVisitor::LP_VISIBLE );

    DFSTraversal traverser;
    const RootNode& rootNode = _dashTree->getDataSource()->getVolumeInformation().rootNode;
    const DashRenderStatus& renderStatus = _dashTree->getRenderStatus();
    switch( renderStatus.getLoadPriority( ))
    {
        case LP_ALL:
        {
            loadVisitor.setLoadPriority( TextureLoaderVisitor::LP_VISIBLE );
            traverser.traverse( rootNode, loadVisitor );
            loadVisitor.setLoadPriority( TextureLoaderVisitor::LP_TEXTURE );
            break;
        }
        case LP_VISIBLE:
            loadVisitor.setLoadPriority( TextureLoaderVisitor::LP_VISIBLE );
            break;
        case LP_TEXTURE:
            loadVisitor.setLoadPriority( TextureLoaderVisitor::LP_TEXTURE );
            break;
        default:
            break;
    }
    traverser.traverse( rootNode, loadVisitor );

    if( !_firstTimeLoaded )
        _firstTimeLoaded = true;
}

void TextureUploadProcessor::runLoop_( )
{
    if( _glWidgetPtr && getGLContext() &&
            livre::GLContext::getCurrent() != glContextPtr_.get( ))
    {
        _glWidgetPtr->getGLContext()->shareContext( getGLContext( ));
        glContextPtr_->makeCurrent();
    }

    if( livre::GLContext::getCurrent() != glContextPtr_.get( ))
        return;

    processorInputPtr_->applyAll( 0 );
#ifdef _ITT_DEBUG_
    __itt_task_begin ( ittTextureLoadDomain, __itt_null, __itt_null, ittTextureComputationTask );
#endif //_ITT_DEBUG_

    const DashRenderStatus& renderStatus = _dashTree->getRenderStatus();
    if( renderStatus.getFrameID() != _currentFrameID )
    {
        _protectUnloading.clear();
        CollectVisiblesVisitor collectVisibles( _dashTree,
                                                _protectUnloading );
        DFSTraversal traverser;
        const RootNode& rootNode =
                _dashTree->getDataSource()->getVolumeInformation().rootNode;
        traverser.traverse( rootNode, collectVisibles );
        _textureCache.setProtectList( _protectUnloading );
        _currentFrameID = renderStatus.getFrameID();
    }

    _checkThreadOperation();
    _loadData();
    processorOutputPtr_->commit( 0 );

#ifdef _ITT_DEBUG_
    __itt_task_end( ittTextureLoadDomain );
#endif //_ITT_DEBUG_
}

void TextureUploadProcessor::_checkThreadOperation()
{
    DashRenderStatus& renderStatus = _dashTree->getRenderStatus();
    ThreadOperation op = renderStatus.getThreadOp();
    if( op != _threadOp )
    {
        _threadOp = op;
        renderStatus.setThreadOp( op );
        processorOutputPtr_->commit( 0 );
    }

    if( _threadOp == TO_EXIT )
        exit( );
}

void TextureLoaderVisitor::visit( DashRenderNode& renderNode, VisitState& state )
{
    const LODNode& lodNode = renderNode.getLODNode();
    if( !lodNode.isValid() )
        return;

    if( lodNode.getRefLevel() > 0)
    {
        switch( loadPriority_ )
        {
            case LP_VISIBLE:
                state.setVisitChild( !renderNode.isVisible() );
                break;
            case LP_TEXTURE:
            {
                DashRenderNode parentNode( getDashTree()->getParentNode( lodNode.getNodeId( )));
                state.setVisitChild( !( !renderNode.isTextureRequested()
                                        && parentNode.isTextureRequested( )));
                break;
            }
        }
    }

    if( ( loadPriority_ == LP_VISIBLE && !renderNode.isVisible() ) ||
        ( loadPriority_ == LP_TEXTURE && !renderNode.isTextureRequested() ) )
    {
        return;
    }

    const ConstCacheObjectPtr texPtr = renderNode.getTextureObject();
    if( texPtr->isLoaded() )
    {
#ifdef _DEBUG_
        LBVERB << "Texture already commited from dash tree:" << lodNode.getNodeId() << std::endl;
#endif //_DEBUG_
        return;
    }

    TextureObject& texture = textureCache_.getNodeTexture( lodNode.getNodeId().getId( ));
    if( !texture.isLoaded() )
    {
        const ConstCacheObjectPtr textureData = renderNode.getTextureDataObject();
        if( textureData->isLoaded() )
        {
#ifdef _ITT_DEBUG_
    __itt_task_begin ( ittTextureLoadDomain, __itt_null, __itt_null, ittTextureLoadTask );
#endif //_ITT_DEBUG_
            TextureObject& lodTexture = textureCache_.getNodeTexture( lodNode.getNodeId().getId( ));
            lodTexture.setTextureDataObject(
                            static_cast< const TextureDataObject * >( textureData.get() ) );
            lodTexture.cacheLoad( );

#ifdef _ITT_DEBUG_
    __itt_task_end( ittTextureLoadDomain );
#endif //_ITT_DEBUG_

            if( loadPriority_ == LP_VISIBLE )
                renderNode.setTextureObject( &lodTexture );

            renderNode.setTextureDataObject( TextureDataObject::getEmptyPtr() );
            processorOutput_->commit( 0 );
        }
#ifdef _DEBUG_
        else
        {
            LBVERB << "Texture data not loaded:" << lodNode.getNodeId() << std::endl;
        }
#endif// _DEBUG_

    }
    else
    {
        if( loadPriority_ == LP_VISIBLE )
        {
            renderNode.setTextureObject( &texture );
            processorOutput_->commit( 0 );
        }
    }

    state.setBreakTraversal( processorInput_->dataWaitingOnInput( 0 ) );
}

}
