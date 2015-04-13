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

#include <livre/core/Data/VolumeDataSource.h>
#include <livre/core/Data/LODNode.h>
#include <livre/core/Dash/DashRenderNode.h>
#include <livre/core/Render/Renderer.h>
#include <livre/core/Render/GLContext.h>
#include <livre/core/Render/GLWidget.h>
#include <livre/Lib/Visitor/DFSTraversal.h>

#include <livre/Lib/Uploaders/DataUploadProcessor.h>
#include <livre/core/DashPipeline/DashProcessorInput.h>
#include <livre/Lib/Cache/RawDataCache.h>
#include <livre/Lib/Cache/TextureDataCache.h>
#include <livre/Lib/Cache/RawDataObject.h>
#include <livre/Lib/Cache/TextureDataObject.h>
#include <livre/Lib/Configuration/VolumeRendererParameters.h>
#include <livre/Lib/Configuration/EFPrefetchAlgorithmParameters.h>
#include <livre/Lib/Visitor/CollectionTraversal.h>
#include <livre/Lib/Cache/LRUCachePolicy.h>

namespace livre
{

#ifdef _ITT_DEBUG_
#include <ittnotify.h>
__itt_domain* ittDataLoadDomain = __itt_domain_create("Data Loading");
__itt_string_handle* ittDataComputationTask = __itt_string_handle_create("Data loading computation");
__itt_string_handle* ittDataLoadTask = __itt_string_handle_create("Data loading task");
#endif // _ITT_DEBUG_

class RawDataLoaderVisitor : public NodeVisitor< dash::NodePtr >
{
public:
    RawDataLoaderVisitor( RawDataCache& rawDataCache,
                          TextureDataCache& textureDataCache,
                          ProcessorInputPtr processorInput,
                          ProcessorOutputPtr processorOutput )
        : rawDataCache_( rawDataCache ),
          textureDataCache_( textureDataCache ),
          processorInput_( processorInput ),
          processorOutput_( processorOutput )
    {}

    void visit( dash::NodePtr dashNode, VisitState& state );

private:
    RawDataCache& rawDataCache_;
    TextureDataCache& textureDataCache_;
    ProcessorInputPtr processorInput_;
    ProcessorOutputPtr processorOutput_;
    lunchbox::Clock _clock;
};

class DepthCollectorVisitor : public NodeVisitor< dash::NodePtr >
{
public:

    DepthCollectorVisitor( RawDataCache& rawDataCache,
                           TextureDataCache& textureDataCache,
                           ProcessorOutputPtr processorOutput,
                           DashNodeVector& refLevelCollection,
                           LoadPriority loadPriority )
        : rawDataCache_( rawDataCache ),
          textureDataCache_( textureDataCache ),
          processorOutput_( processorOutput ),
          refLevelCollection_( refLevelCollection ),
          loadPriority_( loadPriority )
    {}
    void visit( dash::NodePtr node, VisitState& state );

private:

    RawDataCache& rawDataCache_;
    TextureDataCache& textureDataCache_;
    ProcessorOutputPtr processorOutput_;
    DashNodeVector& refLevelCollection_;
    LoadPriority loadPriority_;
};

class DepthSortedDataLoaderVisitor : public NodeVisitor< dash::NodePtr >
{
public:
    DepthSortedDataLoaderVisitor( RawDataCache& rawDataCache,
                                  TextureDataCache& textureDataCache,
                                  ProcessorInputPtr processorInput,
                                  ProcessorOutputPtr processorOutput )
        : rawDataCache_( rawDataCache ),
          textureDataCache_( textureDataCache ),
          processorInput_( processorInput ),
          processorOutput_( processorOutput )
    {}

    void visit( dash::NodePtr lodNode, VisitState& state );

private:

    RawDataCache& rawDataCache_;
    TextureDataCache& textureDataCache_;
    ProcessorInputPtr processorInput_;
    ProcessorOutputPtr processorOutput_;
};

// Sort helper funtion for sorting the textures to load the front texture data first
struct DepthCompare
{
    DepthCompare( const Frustum& frustum )
        : frustum_( frustum ) { }

    bool operator()( dash::NodePtr node1,
                     dash::NodePtr node2 )
    {
        DashRenderNode renderNode1( node1 );
        DashRenderNode renderNode2( node2 );

        const LODNode& lodNode1 = renderNode1.getLODNode();
        const LODNode& lodNode2 = renderNode2.getLODNode();

        const float depth1 = ( frustum_.getEyeCoords() - lodNode1.getWorldBox().getCenter() ).length( );
        const float depth2 = ( frustum_.getEyeCoords() - lodNode2.getWorldBox().getCenter() ).length( );

        return  depth1 < depth2;
    }
    const Frustum& frustum_;
};

DataUploadProcessor::DataUploadProcessor( RawDataCache& rawDataCache,
                                          TextureDataCache& textureDataCache )
    : rawDataCache_( rawDataCache ),
      textureDataCache_( textureDataCache ),
      currentFrameID_( 0 ),
      threadOp_( TO_NONE )
{
}

void DataUploadProcessor::setDashTree( dash::NodePtr dashTree )
{
    dashTree_ = dashTree;
}

void DataUploadProcessor::setGLWidget( GLWidgetPtr glWidgetPtr )
{
    glWidgetPtr_ = glWidgetPtr;
}

bool DataUploadProcessor::initializeThreadRun_()
{
    if( ( !DashProcessor::initializeThreadRun_( ) ) ||
        ( !glContextPtr_.get() ) ||
        ( !glWidgetPtr_.get() || !glWidgetPtr_->getGLContext().get() ) )
    {
        return false;
    }

    glWidgetPtr_->getGLContext()->shareContext( getGLContext() );
    glContextPtr_->makeCurrent();

    VolumeDataSourcePtr dataSource = rawDataCache_.getDataSource();
    dataSource->initializeGL();

    return true;
}

void DataUploadProcessor::runLoop_( )
{
    processorInputPtr_->applyAll( 0 );

#ifdef _ITT_DEBUG_
    __itt_task_begin ( ittDataLoadDomain, __itt_null, __itt_null, ittDataComputationTask );
#endif //_ITT_DEBUG_

    checkThreadOperation_( );
    loadData_( );

#ifdef _ITT_DEBUG_
    __itt_task_end( ittDataLoadDomain );
#endif //_ITT_DEBUG_

}

bool DataUploadProcessor::loadPrioritizedData_( const Frustum& frustum,
                                                const LoadPriority priority )
{

    DashRenderNode rootRenderNode( dashTree_ );
    DashNodeVector dashNodeList;
    DepthCollectorVisitor depthCollectorVisitor( rawDataCache_,
                                                 textureDataCache_,
                                                 processorOutputPtr_,
                                                 dashNodeList,
                                                 priority );
    traverser_.traverse( dashTree_, depthCollectorVisitor );

    std::sort( dashNodeList.begin( ), dashNodeList.end( ),
               DepthCompare( frustum ));
    CollectionTraversal< DashNodeVector > collectionTraverser;
    rootRenderNode.rootSetTreePriority_( priority );
    DepthSortedDataLoaderVisitor refLevelDataLoaderVisitor( rawDataCache_,
                                                            textureDataCache_,
                                                            processorInputPtr_,
                                                          processorOutputPtr_ );
    return collectionTraverser.traverse( dashNodeList,
                                         refLevelDataLoaderVisitor );
}

void DataUploadProcessor::loadData_()
{
    DashRenderNode rootRenderNode( dashTree_ );

    const Frustum& frustum = rootRenderNode.rootGetFrustum_();
    currentFrameID_ = rootRenderNode.rootGetFrameID_();

    DFSTraversal traverser;

    loadPrioritizedData_( frustum, LP_VISIBLE );
    processorOutputPtr_->commit( 0 );

    loadPrioritizedData_( frustum, LP_TEXTURE );
    processorOutputPtr_->commit( 0 );

    RawDataLoaderVisitor loadVisitor( rawDataCache_,
                                      textureDataCache_,
                                      processorInputPtr_,
                                      processorOutputPtr_ );

    traverser.traverse( dashTree_, loadVisitor );
    processorOutputPtr_->commit( 0 );
}

void DataUploadProcessor::checkThreadOperation_( )
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
        exit();
}

void RawDataLoaderVisitor::visit( dash::NodePtr dashNode, VisitState& state )
{
    DashRenderNode renderNode( dashNode );
    const LODNode& node = renderNode.getLODNode();

    if( !node.isValid() )
        return;

    state.setBreakTraversal( processorInput_->dataWaitingOnInput( 0 ));
    if( renderNode.getLODNode().getRefLevel() > 0 )
        state.setVisitChild( ( renderNode.isDataRequested() ||
                               !renderNode.getParent().isDataRequested( )));

    if( !renderNode.isDataRequested() )
        return;

    const ConstCacheObjectPtr texture = renderNode.getTextureObject();
    if( texture->isLoaded( ))
        return;

    const TextureDataObject& tData = textureDataCache_.getNodeTextureData(
                                         node.getNodeId().getId( ));
    if( tData.isLoaded( ))
        return;

#ifdef _ITT_DEBUG_
    __itt_task_begin( ittDataLoadDomain, __itt_null, __itt_null,
                      ittDataLoadTask );
#endif //_ITT_DEBUG_
    RawDataObject& lodData = rawDataCache_.getNodeData( node.getNodeId().getId( ));
    lodData.cacheLoad( );

    TextureDataObject& textureData =
        textureDataCache_.getNodeTextureData( node.getNodeId().getId( ));
    textureData.setRawData( &lodData );
    textureData.cacheLoad( );
    if( _clock.getTime64() > 1000 ) // commit once every second
    {
        _clock.reset();
        processorOutput_->commit( 0 );
    }
#ifdef _ITT_DEBUG_
    __itt_task_end( ittDataLoadDomain );
#endif //_ITT_DEBUG_
}

void DepthCollectorVisitor::visit( dash::NodePtr dashNode, VisitState& state )
{
    DashRenderNode renderNode( dashNode );
    const LODNode& lodNode = renderNode.getLODNode();

    if( !lodNode.isValid() )
        return;

    if( lodNode.getRefLevel() > 0 )
    {
        switch( loadPriority_ )
        {
        case LP_VISIBLE:
            state.setVisitChild( !renderNode.isVisible() );
            break;
        case LP_TEXTURE:
            state.setVisitChild( renderNode.isTextureRequested() ||
                                 !renderNode.getParent().isTextureRequested( ));
            break;
        default:
            break;
        }
    }

    if(( loadPriority_ == LP_VISIBLE && !renderNode.isVisible( )) ||
       ( loadPriority_ == LP_TEXTURE && !renderNode.isTextureRequested( )))
    {
        return;
    }

    const ConstCacheObjectPtr texture = renderNode.getTextureObject();
    if( texture->isLoaded() )
        return;

    const ConstCacheObjectPtr tData = renderNode.getTextureDataObject();
    if( tData->isLoaded() )
        return;

    // For to create cache object
    rawDataCache_.getNodeData( lodNode.getNodeId().getId( ));
    TextureDataObject& textureData =
            textureDataCache_.getNodeTextureData( lodNode.getNodeId().getId( ));
    if( textureData.isLoaded() )
    {
        renderNode.setTextureDataObject( &textureData );
        processorOutput_->commit( 0 );
        return;
    }

    refLevelCollection_.push_back( dashNode );
}

void DepthSortedDataLoaderVisitor::visit( dash::NodePtr node, VisitState& state)
{
    if( !node )
        return;

    DashRenderNode renderNode( node );
    const LODNode& lodNode = renderNode.getLODNode();

#ifdef _ITT_DEBUG_
    __itt_task_begin( ittDataLoadDomain, __itt_null, __itt_null, ittDataLoadTask );
#endif //_ITT_DEBUG_

    RawDataObject& lodData =
            static_cast< const RawDataCache& >( rawDataCache_ ).getNodeData(
                lodNode.getNodeId().getId( ));
    lodData.cacheLoad( );

    TextureDataObject& textureData =
            static_cast< const TextureDataCache& >
            ( textureDataCache_ ).getNodeTextureData( lodNode.getNodeId().getId( ));
    textureData.setRawData( &lodData );
    textureData.cacheLoad();

#ifdef _ITT_DEBUG_
    __itt_task_end( ittDataLoadDomain );
#endif //_ITT_DEBUG_

    renderNode.setTextureDataObject( &textureData );

#ifdef _DEBUG_
    const ConstCacheObjectPtr tData = renderNode.getTextureDataObject();
    if( !tData->isLoaded() )
    {
        LBERROR << "Texture data loaded but no in the render node : "
                << textureData.getLODNode()->getNodeId().getId();
                << std::endl;
    }
#endif //_DEBUG_

    processorOutput_->commit( 0 );
    state.setBreakTraversal( processorInput_->dataWaitingOnInput( 0 ) );

}

}
