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

#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Render/GLContext.h>
#include <livre/core/Render/GLWidget.h>
#include <livre/core/Render/Renderer.h>
#include <livre/core/Render/gl.h>

namespace livre
{

#ifdef _ITT_DEBUG_
#include "ittnotify.h"
__itt_domain* ittTextureLoadDomain = __itt_domain_create("Texture Loading");
__itt_string_handle* ittTextureComputationTask = __itt_string_handle_create("Texture loading computation");
__itt_string_handle* ittTextureLoadTask = __itt_string_handle_create("Texture loading task");
#endif // _ITT_DEBUG_


// Dont forget to apply cache policy after each traversal
class TextureLoaderVisitor : public NodeVisitor< dash::NodePtr >
{
public:

    enum LoadPriority
    {
        LP_VISIBLE,
        LP_TEXTURE
    };

    TextureLoaderVisitor( TextureCache& textureCache,
                          ProcessorInputPtr processorInput,
                          ProcessorOutputPtr processorOutput,
                          const LoadPriority loadPriority  )
        : textureCache_( textureCache ),
          processorInput_( processorInput ),
          processorOutput_( processorOutput ),
          loadPriority_( loadPriority )
    {}

    void visit( dash::NodePtr dashNode, VisitState& state );
    void setLoadPriority( LoadPriority loadPriority ) { loadPriority_ = loadPriority; }

private:

    TextureCache& textureCache_;
    ProcessorInputPtr processorInput_;
    ProcessorOutputPtr processorOutput_;
    LoadPriority loadPriority_;
};

class CollectVisiblesVisitor : public NodeVisitor< dash::NodePtr >
{
public:
    CollectVisiblesVisitor( CacheIdSet& currentVisibleSet )
     : currentVisibleSet_( currentVisibleSet ) {}

    void visit( dash::NodePtr dashNode, VisitState& state )
    {
        DashRenderNode renderNode( dashNode );
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

TextureUploadProcessor::TextureUploadProcessor( TextureCache &textureCache )
    : textureCache_( textureCache ),
      currentFrameID_( 0 ),
      threadOp_( TO_NONE ),
      firstTimeLoaded_( false )
{
}

void TextureUploadProcessor::setGLWidget( GLWidgetPtr glWidgetPtr )
{
    glWidgetPtr_ = glWidgetPtr;
}

bool TextureUploadProcessor::initializeThreadRun_()
{
    setName( "TextureUpload" );
    if( !DashProcessor::initializeThreadRun_( ) )
        return false;

    if( !glContextPtr_.get() )
        return false;

    if( !glWidgetPtr_.get() || !glWidgetPtr_->getGLContext().get() )
        return false;

    glWidgetPtr_->getGLContext()->shareContext( getGLContext() );
    glContextPtr_->makeCurrent();
    return true;
}

bool TextureUploadProcessor::onPreCommit_( const uint32_t outputConnection LB_UNUSED )
{
    if( !firstTimeLoaded_ )
        return false;

    return true;
}

void TextureUploadProcessor::onPostCommit_( const uint32_t connection LB_UNUSED, const CommitState state LB_UNUSED )
{
    if( state != CS_NOCHANGE )
    {
        glFinish( );
    }

    if( state == CS_COMMITED )
    {
        glWidgetPtr_->update();
    }
}

void TextureUploadProcessor::setDashTree( dash::NodePtr dashTree )
{
    dashTree_ = dashTree;
}

void TextureUploadProcessor::loadData_()
{
    TextureLoaderVisitor loadVisitor( textureCache_,
                                      processorInputPtr_,
                                      processorOutputPtr_,
                                      TextureLoaderVisitor::LP_VISIBLE );

    DFSTraversal traverser;
    switch( DashRenderNode( dashTree_ ).rootGetLoadPriority_() )
    {
        case LP_ALL:
            loadVisitor.setLoadPriority( TextureLoaderVisitor::LP_VISIBLE );
            traverser.traverse( dashTree_, loadVisitor );
            loadVisitor.setLoadPriority( TextureLoaderVisitor::LP_TEXTURE );
            break;
        case LP_VISIBLE:
            loadVisitor.setLoadPriority( TextureLoaderVisitor::LP_VISIBLE );
            break;
        case LP_TEXTURE:
            loadVisitor.setLoadPriority( TextureLoaderVisitor::LP_TEXTURE );
            break;
        default:
            break;
    }
    traverser.traverse( dashTree_, loadVisitor );

    if( !firstTimeLoaded_ )
        firstTimeLoaded_ = true;
}

void TextureUploadProcessor::runLoop_( )
{
    processorInputPtr_->applyAll( 0 );
#ifdef _ITT_DEBUG_
    __itt_task_begin ( ittTextureLoadDomain, __itt_null, __itt_null, ittTextureComputationTask );
#endif //_ITT_DEBUG_

    DashRenderNode rootRenderNode( dashTree_ );
    if( rootRenderNode.rootGetFrameID_() != currentFrameID_ )
    {
        protectUnloading_.clear();
        CollectVisiblesVisitor collectVisibles( protectUnloading_ );
        DFSTraversal traverser;
        traverser.traverse( dashTree_, collectVisibles );
        textureCache_.setProtectList( protectUnloading_ );
        currentFrameID_ = rootRenderNode.rootGetFrameID_();
    }

    checkThreadOperation_( );
    loadData_( );
    processorOutputPtr_->commit( 0 );

#ifdef _ITT_DEBUG_
    __itt_task_end( ittTextureLoadDomain );
#endif //_ITT_DEBUG_
}

void TextureUploadProcessor::checkThreadOperation_( )
{
    DashRenderNode rootNode( dashTree_ );
    ThreadOperation op = rootNode.rootGetThreadOp_();
    if( op != threadOp_ )
    {
        threadOp_ = op;
        rootNode.rootSetThreadOp_( op );
        processorOutputPtr_->commit( 0 );
    }

    if( threadOp_ == TO_EXIT )
    {
        exit( );
    }
}

void TextureLoaderVisitor::visit( dash::NodePtr dashNode, VisitState& state )
{
    DashRenderNode renderNode( dashNode );

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
            state.setVisitChild( !( !renderNode.isTextureRequested() && renderNode.getParent().isTextureRequested() ) );
            break;
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
