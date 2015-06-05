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
                           DashNodeVector& refLevelCollection )
        : RenderNodeVisitor( dashTree ),
          rawDataCache_( rawDataCache ),
          textureDataCache_( textureDataCache ),
          processorOutput_( processorOutput ),
          refLevelCollection_( refLevelCollection )
    {}
    void visit( DashRenderNode& node, VisitState& state ) final;

private:

    RawDataCache& rawDataCache_;
    TextureDataCache& textureDataCache_;
    ProcessorOutputPtr processorOutput_;
    DashNodeVector& refLevelCollection_;
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
                                          GLContextPtr shareContext,
                                          GLContextPtr context,
                                          RawDataCache& rawDataCache,
                                          TextureDataCache& textureDataCache )
    : GLContextTrait( context )
    , _dashTree( dashTree )
    , _shareContext( shareContext )
    , _rawDataCache( rawDataCache )
    , _textureDataCache( textureDataCache )
    , _currentFrameID( 0 )
    , _threadOp( TO_NONE )
{
    setDashContext( dashTree->createContext() );
}

bool DataUploadProcessor::initializeThreadRun_()
{
    setName( "DataUp" );
    return DashProcessor::initializeThreadRun_();
}

void DataUploadProcessor::runLoop_( )
{
    LBASSERT( getGLContext( ));
    if( GLContext::getCurrent() != getGLContext().get( ))
    {
        _shareContext->shareContext( getGLContext( ));
        getGLContext()->makeCurrent();

        VolumeDataSourcePtr dataSource = _rawDataCache.getDataSource();
        dataSource->initializeGL();
    }

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

void DataUploadProcessor::_loadData()
{
    const DashRenderStatus& renderStatus = _dashTree->getRenderStatus();

    const Frustum& frustum = renderStatus.getFrustum();
    _currentFrameID = renderStatus.getFrameID();


    DashNodeVector dashNodeList;
    DepthCollectorVisitor depthCollectorVisitor( _dashTree,
                                                 _rawDataCache,
                                                 _textureDataCache,
                                                 processorOutputPtr_,
                                                 dashNodeList );

    const RootNode& rootNode = _dashTree->getDataSource()->getVolumeInformation().rootNode;

    DFSTraversal traverser;
    traverser.traverse( rootNode, depthCollectorVisitor );

    std::sort( dashNodeList.begin( ), dashNodeList.end( ), DepthCompare( frustum ));
    CollectionTraversal collectionTraverser;
    DepthSortedDataLoaderVisitor refLevelDataLoaderVisitor( _dashTree,
                                                            _rawDataCache,
                                                            _textureDataCache,
                                                            processorInputPtr_,
                                                            processorOutputPtr_ );
    collectionTraverser.traverse( dashNodeList, refLevelDataLoaderVisitor );


    processorOutputPtr_->commit( 0 );

    RawDataLoaderVisitor loadVisitor( _dashTree,
                                      _rawDataCache,
                                      _textureDataCache,
                                      processorInputPtr_,
                                      processorOutputPtr_ );

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

    if( !renderNode.isVisible())
        return;

    state.setVisitChild( false );

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

    const bool isVisible = renderNode.isVisible();
    state.setVisitChild( !isVisible );

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
