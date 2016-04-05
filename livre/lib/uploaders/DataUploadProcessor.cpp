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

#include <livre/core/data/DataSource.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/dash/DashRenderNode.h>
#include <livre/core/dash/DashTree.h>
#include <livre/core/render/Renderer.h>
#include <livre/core/render/GLContext.h>
#include <livre/core/visitor/RenderNodeVisitor.h>
#include <livre/lib/visitor/DFSTraversal.h>

#include <livre/lib/uploaders/DataUploadProcessor.h>
#include <livre/core/dashpipeline/DashProcessorInput.h>
#include <livre/lib/cache/TextureDataCache.h>
#include <livre/lib/cache/TextureDataObject.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>
#include <livre/lib/visitor/CollectionTraversal.h>
#include <livre/lib/visitor/CollectionTraversal.h>

namespace livre
{

#ifdef _ITT_DEBUG_
#include <ittnotify.h>
__itt_domain* ittDataLoadDomain = __itt_domain_create("Data Loading");
__itt_string_handle* ittDataComputationTask = __itt_string_handle_create("Data loading computation");
__itt_string_handle* ittDataLoadTask = __itt_string_handle_create("Data loading task");
#endif // _ITT_DEBUG_

class DataLoaderVisitor : public RenderNodeVisitor
{
public:
    DataLoaderVisitor( DashTree& dashTree, TextureDataCache& textureDataCache,
                       ProcessorInputPtr processorInput,
                       ProcessorOutputPtr processorOutput )
        : RenderNodeVisitor( dashTree ),
          _cache( textureDataCache ),
          _input( processorInput ),
          _output( processorOutput )
    {}

    void visit( DashRenderNode& renderNode, VisitState& state );

private:
    TextureDataCache& _cache;
    ProcessorInputPtr _input;
    ProcessorOutputPtr _output;
    lunchbox::Clock _clock;
};

class DepthCollectorVisitor : public RenderNodeVisitor
{
public:
    DepthCollectorVisitor( DashTree& dashTree,
                           TextureDataCache& textureDataCache,
                           ProcessorOutputPtr processorOutput,
                           DashNodeVector& refLevelCollection )
        : RenderNodeVisitor( dashTree ),
          _cache( textureDataCache ),
          _output( processorOutput ),
          _collection( refLevelCollection )
    {}
    void visit( DashRenderNode& node, VisitState& state ) final;

private:
    TextureDataCache& _cache;
    ProcessorOutputPtr _output;
    DashNodeVector& _collection;
};

class DepthSortedDataLoaderVisitor : public RenderNodeVisitor
{
public:
    DepthSortedDataLoaderVisitor( DashTree& dashTree,
                                  TextureDataCache& textureDataCache,
                                  ProcessorInputPtr processorInput,
                                  ProcessorOutputPtr processorOutput )
        : RenderNodeVisitor( dashTree )
        , _cache( textureDataCache )
        , _input( processorInput )
        , _output( processorOutput )
    {}

    void visit( DashRenderNode& renderNode, VisitState& state ) final;
private:
    TextureDataCache& _cache;
    ProcessorInputPtr _input;
    ProcessorOutputPtr _output;
};

// Sort helper function for sorting the textures to load the front texture data first
struct DepthCompare
{
    explicit DepthCompare( const Frustum& frustum )
        : frustum_( frustum ) { }

    bool operator()( dash::NodePtr node1,
                     dash::NodePtr node2 )
    {
        DashRenderNode renderNode1( node1 );
        DashRenderNode renderNode2( node2 );

        const LODNode& lodNode1 = renderNode1.getLODNode();
        const LODNode& lodNode2 = renderNode2.getLODNode();

        const float depth1 = ( frustum_.getEyePos() -
                               lodNode1.getWorldBox().getCenter( )).length();
        const float depth2 = ( frustum_.getEyePos() -
                               lodNode2.getWorldBox().getCenter( )).length();
        return  depth1 < depth2;
    }
    const Frustum& frustum_;
};

DataUploadProcessor::DataUploadProcessor( DashTree& dashTree,
                                          GLContextPtr shareContext,
                                          TextureDataCache& textureDataCache )
    : _dashTree( dashTree )
    , _glContext( shareContext->clone( ))
    , _sharedContext( shareContext )
    , _textureDataCache( textureDataCache )
    , _currentFrameID( 0 )
    , _threadOp( TO_NONE )
{
    setDashContext( dashTree.createContext( ));
}

bool DataUploadProcessor::initializeThreadRun_()
{
    setName( "DataUp" );
    DataSource& dataSource = _textureDataCache.getDataSource();
    dataSource.initializeGL();
    return DashProcessor::initializeThreadRun_();
}

void DataUploadProcessor::runLoop_()
{
    LBASSERT( _glContext );
    if( GLContext::getCurrent() != _glContext.get( ))
    {
        _glContext->share( *_sharedContext );
        _glContext->makeCurrent();
    }

    processorInputPtr_->applyAll( CONNECTION_ID );

#ifdef _ITT_DEBUG_
    __itt_task_begin ( ittDataLoadDomain, __itt_null, __itt_null, ittDataComputationTask );
#endif //_ITT_DEBUG_

    _checkThreadOperation();
    _loadData();

#ifdef _ITT_DEBUG_
    __itt_task_end( ittDataLoadDomain );
#endif //_ITT_DEBUG_

}

void DataUploadProcessor::_loadData()
{
    const DashRenderStatus& renderStatus = _dashTree.getRenderStatus();

    const Frustum& frustum = renderStatus.getFrustum();
    _currentFrameID = renderStatus.getFrameID();

    DashNodeVector dashNodeList;
    DepthCollectorVisitor depthCollectorVisitor( _dashTree,
                                                 _textureDataCache,
                                                 processorOutputPtr_,
                                                 dashNodeList );

    const RootNode& rootNode = _dashTree.getDataSource().getVolumeInfo().rootNode;

    DFSTraversal traverser;
    traverser.traverse( rootNode, depthCollectorVisitor, _currentFrameID );

    std::sort( dashNodeList.begin( ), dashNodeList.end( ),
               DepthCompare( frustum ));
    CollectionTraversal collectionTraverser;
    DepthSortedDataLoaderVisitor dataLoader( _dashTree, _textureDataCache,
                                             processorInputPtr_,
                                             processorOutputPtr_ );
    collectionTraverser.traverse( dashNodeList, dataLoader );
    processorOutputPtr_->commit( CONNECTION_ID );

    DataLoaderVisitor loadVisitor( _dashTree, _textureDataCache,
                                   processorInputPtr_, processorOutputPtr_ );

    traverser.traverse( rootNode, loadVisitor, _currentFrameID );
    processorOutputPtr_->commit( CONNECTION_ID );
}

void DataUploadProcessor::_checkThreadOperation()
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

void DataLoaderVisitor::visit( DashRenderNode& renderNode, VisitState& state )
{
    const LODNode& node = renderNode.getLODNode();

    if( !node.isValid() )
        return;

    state.setBreakTraversal( _input->dataWaitingOnInput( CONNECTION_ID ));

    if( !renderNode.isInFrustum( ))
    {
        state.setVisitChild( false );
        return;
    }

    if( !renderNode.isLODVisible( ))
        return;

    state.setVisitChild( false );

    const ConstCacheObjectPtr texture = renderNode.getTextureObject();
    if( texture && texture->isLoaded( ))
        return;

    const ConstCacheObjectPtr tData = _cache.get( node.getNodeId().getId( ));
    if( tData->isLoaded( ))
        return;

#ifdef _ITT_DEBUG_
    __itt_task_begin( ittDataLoadDomain, __itt_null, __itt_null,
                      ittDataLoadTask );
#endif //_ITT_DEBUG_
    CacheObjectPtr textureData = _cache.load( node.getNodeId().getId( ));
    if( _clock.getTime64() > 1000 ) // commit once every second
    {
        _clock.reset();
        _output->commit( CONNECTION_ID );
    }
#ifdef _ITT_DEBUG_
    __itt_task_end( ittDataLoadDomain );
#endif //_ITT_DEBUG_
}

void DepthCollectorVisitor::visit( DashRenderNode& renderNode, VisitState& state )
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

    const ConstCacheObjectPtr texture = renderNode.getTextureObject();
    if( texture && texture->isLoaded( ))
        return;

    const ConstCacheObjectPtr tData = renderNode.getTextureDataObject();
    if( tData && tData->isLoaded( ))
        return;

    // Triggers creation of the cache object.
    const ConstCacheObjectPtr textureData = _cache.get( lodNode.getNodeId().getId( ));
    if( textureData && textureData->isLoaded( ))
    {
        renderNode.setTextureDataObject( textureData );
        _output->commit( CONNECTION_ID );
        return;
    }

    _collection.push_back( renderNode.getDashNode());
}

void DepthSortedDataLoaderVisitor::visit( DashRenderNode& renderNode,
                                          VisitState& state )
{
    const LODNode& lodNode = renderNode.getLODNode();

#ifdef _ITT_DEBUG_
    __itt_task_begin( ittDataLoadDomain, __itt_null, __itt_null,
                      ittDataLoadTask );
#endif //_ITT_DEBUG_

    CacheObjectPtr textureData = _cache.load( lodNode.getNodeId().getId( ));
#ifdef _ITT_DEBUG_
    __itt_task_end( ittDataLoadDomain );
#endif //_ITT_DEBUG_

    renderNode.setTextureDataObject( textureData );

#ifdef _DEBUG_
    const ConstCacheObjectPtr tData = renderNode.getTextureDataObject();
    if( !tData->isLoaded() )
    {
        LBERROR << "Texture data loaded but no in the render node : "
                << textureData.getLODNode()->getNodeId().getId();
                << std::endl;
    }
#endif //_DEBUG_

    _output->commit( CONNECTION_ID );
    state.setBreakTraversal( _input->dataWaitingOnInput( CONNECTION_ID ));

}

}
