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
#include <livre/core/Dash/DashTree.h>
#include <livre/core/Render/Renderer.h>
#include <livre/core/Render/GLContext.h>
#include <livre/core/Render/GLWidget.h>
#include <livre/core/Visitor/RenderNodeVisitor.h>
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
#include <livre/Lib/Visitor/CollectionTraversal.h>

namespace livre
{

#ifdef _ITT_DEBUG_
#include <ittnotify.h>
__itt_domain* ittDataLoadDomain = __itt_domain_create("Data Loading");
__itt_string_handle* ittDataComputationTask = __itt_string_handle_create("Data loading computation");
__itt_string_handle* ittDataLoadTask = __itt_string_handle_create("Data loading task");
#endif // _ITT_DEBUG_

class RawDataLoaderVisitor : public RenderNodeVisitor
{
public:
    RawDataLoaderVisitor( DashTreePtr dashTree,
                          RawDataCache& rawDataCache,
                          TextureDataCache& textureDataCache,
                          ProcessorInputPtr processorInput,
                          ProcessorOutputPtr processorOutput )
        : RenderNodeVisitor( dashTree ),
          rawDataCache_( rawDataCache ),
          textureDataCache_( textureDataCache ),
          processorInput_( processorInput ),
          processorOutput_( processorOutput )
    {}

    void visit( DashRenderNode& renderNode, VisitState& state );

private:
    RawDataCache& rawDataCache_;
    TextureDataCache& textureDataCache_;
    ProcessorInputPtr processorInput_;
    ProcessorOutputPtr processorOutput_;
    lunchbox::Clock _clock;
};

class DepthCollectorVisitor : public RenderNodeVisitor
{
public:

    DepthCollectorVisitor( DashTreePtr dashTree,
                           RawDataCache& rawDataCache,
                           TextureDataCache& textureDataCache,
                           ProcessorOutputPtr processorOutput,
                           DashNodeVector& refLevelCollection,
                           LoadPriority loadPriority )
        : RenderNodeVisitor( dashTree ),
          rawDataCache_( rawDataCache ),
          textureDataCache_( textureDataCache ),
          processorOutput_( processorOutput ),
          refLevelCollection_( refLevelCollection ),
          loadPriority_( loadPriority )
    {}
    void visit( DashRenderNode& node, VisitState& state ) final;

private:

    RawDataCache& rawDataCache_;
    TextureDataCache& textureDataCache_;
    ProcessorOutputPtr processorOutput_;
    DashNodeVector& refLevelCollection_;
    LoadPriority loadPriority_;
};

class DepthSortedDataLoaderVisitor : public RenderNodeVisitor
{
public:
    DepthSortedDataLoaderVisitor( DashTreePtr dashTree,
                                  RawDataCache& rawDataCache,
                                  TextureDataCache& textureDataCache,
                                  ProcessorInputPtr processorInput,
                                  ProcessorOutputPtr processorOutput )
        : RenderNodeVisitor( dashTree ),
          rawDataCache_( rawDataCache ),
          textureDataCache_( textureDataCache ),
          processorInput_( processorInput ),
          processorOutput_( processorOutput )
    {}

    void visit( DashRenderNode& renderNode, VisitState& state ) final;

private:

    RawDataCache& rawDataCache_;
    TextureDataCache& textureDataCache_;
    ProcessorInputPtr processorInput_;
    ProcessorOutputPtr processorOutput_;
};

// Sort helper function for sorting the textures to load the front texture data first
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

        const float depth1 =
                ( frustum_.getEyeCoords() - lodNode1.getWorldBox().getCenter( )).length();
        const float depth2 =
                ( frustum_.getEyeCoords() - lodNode2.getWorldBox().getCenter( )).length();

        return  depth1 < depth2;
    }
    const Frustum& frustum_;
};

DataUploadProcessor::DataUploadProcessor( DashTreePtr dashTree,
                                          RawDataCache& rawDataCache,
                                          TextureDataCache& textureDataCache )
    : _dashTree( dashTree ),
      _rawDataCache( rawDataCache ),
      _textureDataCache( textureDataCache ),
      _currentFrameID( 0 ),
      _threadOp( TO_NONE )
{
    setDashContext( dashTree->createContext() );
}

void DataUploadProcessor::setGLWidget( GLWidgetPtr glWidgetPtr )
{
    _glWidgetPtr = glWidgetPtr;
}

bool DataUploadProcessor::initializeThreadRun_()
{
    setName( "DataUpload" );
    if( !DashProcessor::initializeThreadRun_( ))
        return false;


    return true;
}

void DataUploadProcessor::runLoop_( )
{
    if( _glWidgetPtr && getGLContext() &&
            livre::GLContext::getCurrent() != glContextPtr_.get( ))
    {
        _glWidgetPtr->getGLContext()->shareContext( getGLContext( ));
        glContextPtr_->makeCurrent();

        VolumeDataSourcePtr dataSource = _rawDataCache.getDataSource();
        dataSource->initializeGL();
    }

    if( livre::GLContext::getCurrent() != glContextPtr_.get( ))
        return;

    processorInputPtr_->applyAll( 0 );

#ifdef _ITT_DEBUG_
    __itt_task_begin ( ittDataLoadDomain, __itt_null, __itt_null, ittDataComputationTask );
#endif //_ITT_DEBUG_

    _checkThreadOperation( );
    _loadData( );

#ifdef _ITT_DEBUG_
    __itt_task_end( ittDataLoadDomain );
#endif //_ITT_DEBUG_

}

bool DataUploadProcessor::_loadPrioritizedData( const Frustum& frustum,
                                                const LoadPriority priority )
{
    DashNodeVector dashNodeList;
    DepthCollectorVisitor depthCollectorVisitor( _dashTree,
                                                 _rawDataCache,
                                                 _textureDataCache,
                                                 processorOutputPtr_,
                                                 dashNodeList,
                                                 priority );
    const RootNode& rootNode = _dashTree->getDataSource()->getVolumeInformation().rootNode;
    DFSTraversal traverser;
    traverser.traverse( rootNode, depthCollectorVisitor );

    std::sort( dashNodeList.begin( ), dashNodeList.end( ),
               DepthCompare( frustum ));
    CollectionTraversal collectionTraverser;
    DashRenderStatus& renderStatus = _dashTree->getRenderStatus();
    renderStatus.setLoadPriority( priority );
    DepthSortedDataLoaderVisitor refLevelDataLoaderVisitor( _dashTree,
                                                            _rawDataCache,
                                                            _textureDataCache,
                                                            processorInputPtr_,
                                                            processorOutputPtr_ );
    return collectionTraverser.traverse( dashNodeList,
                                         refLevelDataLoaderVisitor );
}

void DataUploadProcessor::_loadData()
{
    const DashRenderStatus& renderStatus = _dashTree->getRenderStatus();

    const Frustum& frustum = renderStatus.getFrustum();
    _currentFrameID = renderStatus.getFrameID();

    DFSTraversal traverser;

    _loadPrioritizedData( frustum, LP_VISIBLE );
    processorOutputPtr_->commit( 0 );

    _loadPrioritizedData( frustum, LP_TEXTURE );
    processorOutputPtr_->commit( 0 );

    RawDataLoaderVisitor loadVisitor( _dashTree,
                                      _rawDataCache,
                                      _textureDataCache,
                                      processorInputPtr_,
                                      processorOutputPtr_ );

    const RootNode& rootNode = _dashTree->getDataSource()->getVolumeInformation().rootNode;
    traverser.traverse( rootNode, loadVisitor );
    processorOutputPtr_->commit( 0 );
}

void DataUploadProcessor::_checkThreadOperation()
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
        exit();
}

void RawDataLoaderVisitor::visit( DashRenderNode& renderNode, VisitState& state )
{
    const LODNode& node = renderNode.getLODNode();

    if( !node.isValid() )
        return;

    state.setBreakTraversal( processorInput_->dataWaitingOnInput( 0 ));

    DashRenderNode parentNode( getDashTree()->getParentNode(node.getNodeId( )));

    if( renderNode.getLODNode().getRefLevel() > 0 )
        state.setVisitChild( ( renderNode.isDataRequested() ||
                               !parentNode.isDataRequested( )));

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

void DepthCollectorVisitor::visit( DashRenderNode& renderNode, VisitState& state )
{
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
        {
            DashRenderNode parentNode( getDashTree()->getParentNode(lodNode.getNodeId( )));
            state.setVisitChild( renderNode.isTextureRequested() ||
                                 !parentNode.isTextureRequested( ));
            break;
        }
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
    if( texture->isLoaded( ))
        return;

    const ConstCacheObjectPtr tData = renderNode.getTextureDataObject();
    if( tData->isLoaded( ))
        return;

    // Triggers creation of the cache object.
    rawDataCache_.getNodeData( lodNode.getNodeId().getId( ));
    TextureDataObject& textureData =
            textureDataCache_.getNodeTextureData( lodNode.getNodeId().getId( ));
    if( textureData.isLoaded() )
    {
        renderNode.setTextureDataObject( &textureData );
        processorOutput_->commit( 0 );
        return;
    }

    refLevelCollection_.push_back( renderNode.getDashNode());
}

void DepthSortedDataLoaderVisitor::visit( DashRenderNode& renderNode, VisitState& state )
{
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
