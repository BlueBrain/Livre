
/* Copyright (c) 2006-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya <maxmah@gmail.com>
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include <livre/eq/Channel.h>
#include <livre/eq/Config.h>
#include <livre/eq/Event.h>
#include <livre/eq/Error.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/FrameGrabber.h>
#include <livre/eq/Node.h>
#include <livre/eq/Pipe.h>
#include <livre/eq/render/EqContext.h>
#include <livre/eq/render/RayCastRenderer.h>
#include <livre/eq/settings/CameraSettings.h>
#include <livre/eq/settings/FrameSettings.h>
#include <livre/eq/settings/RenderSettings.h>
#include <livre/eq/Window.h>

#include <livre/lib/cache/TextureCache.h>
#include <livre/lib/cache/TextureDataCache.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/lib/render/AvailableSetGenerator.h>
#include <livre/lib/render/ScreenSpaceLODEvaluator.h>
#include <livre/lib/render/SelectVisibles.h>
#include <livre/lib/visitor/DFSTraversal.h>

#include <livre/core/cache/CacheStatistics.h>
#include <livre/core/dash/DashRenderStatus.h>
#include <livre/core/dash/DashTree.h>
#include <livre/core/dashpipeline/DashProcessorInput.h>
#include <livre/core/dashpipeline/DashProcessorOutput.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/render/FrameInfo.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/RenderBrick.h>

#include <eq/eq.h>
#include <eq/gl.h>

namespace livre
{
const float nearPlane = 0.1f;
const float farPlane = 15.0f;

struct Channel::Impl
{
public:
    explicit Impl( Channel* channel )
          : _channel( channel )
          , _frustum( Matrix4f(), Matrix4f( ))
          , _frameInfo( _frustum, INVALID_FRAME )
    {}

    void initializeFrame()
    {
        _channel->setNearFar( nearPlane, farPlane );

        eq::FrameDataPtr frameData = new eq::FrameData();
        frameData->setBuffers( eq::Frame::BUFFER_COLOR );
        _frame.setFrameData( frameData );
        _frame.setName( std::string( "self." ) + _channel->getName( ));
    }

    ConstFrameDataPtr getFrameData() const
    {
        const livre::Pipe* pipe = static_cast< const livre::Pipe* >( _channel->getPipe( ));
        return pipe->getFrameData();
    }

    void initializeRenderer()
    {
        const uint32_t nSamplesPerRay =
            getFrameData()->getVRParameters().getSamplesPerRay();

        const uint32_t nSamplesPerPixel =
            getFrameData()->getVRParameters().getSamplesPerPixel();

        const livre::Node* node =
                static_cast< livre::Node* >( _channel->getNode( ));

        const livre::DashTree& dashTree = node->getDashTree();
        ConstDataSourcePtr dataSource = dashTree.getDataSource();
        _renderer.reset( new RayCastRenderer(
                                  nSamplesPerRay,
                                  nSamplesPerPixel,
                                  dataSource->getVolumeInfo( )));
    }

    const Frustum& setupFrustum()
    {
        const eq::Matrix4f& modelView = computeModelView();
        const eq::Frustumf& eqFrustum = _channel->getFrustum();
        const eq::Matrix4f& projection = eqFrustum.computePerspectiveMatrix();

        _frustum = Frustum( modelView, projection );
        return _frustum;
    }

    eq::Matrix4f computeModelView() const
    {
        const CameraSettings& cameraSettings =
            getFrameData()->getCameraSettings();
        Matrix4f modelView = cameraSettings.getModelViewMatrix();
        modelView = _channel->getHeadTransform() * modelView;
        return modelView;
    }

    void clearViewport( const eq::PixelViewport &pvp )
    {
        // clear given area
        glScissor( pvp.x, pvp.y, pvp.w, pvp.h );
        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // restore assembly state
        const eq::PixelViewport& channelPvp = _channel->getPixelViewport( );
        glScissor( 0, 0, channelPvp.w, channelPvp.h );
    }

    void generateRenderBricks( const ConstCacheObjects& renderNodes,
                               RenderBricks& renderBricks )
    {

        renderBricks.reserve( renderNodes.size( ));

        livre::Node* node = static_cast< livre::Node* >( _channel->getNode( ));
        livre::DashTree& dashTree = node->getDashTree();

        for( const ConstCacheObjectPtr& cacheObject: renderNodes )
        {
            const ConstTextureObjectPtr texture =
                std::static_pointer_cast< const TextureObject >( cacheObject );

            const LODNode& lodNode =
                    dashTree.getDataSource()->getNode( NodeId( cacheObject->getId( )));

            renderBricks.emplace_back( lodNode, texture->getTextureState( ));
        }
    }

    DashRenderNodes requestData()
    {
        livre::Node* node = static_cast< livre::Node* >( _channel->getNode( ));
        livre::Window* window = static_cast< livre::Window* >( _channel->getWindow( ));
        livre::Pipe* pipe = static_cast< livre::Pipe* >( window->getPipe( ));

        const VolumeRendererParameters& vrParams =
            pipe->getFrameData()->getVRParameters();
        const uint32_t minLOD = vrParams.getMinLOD();
        const uint32_t maxLOD = vrParams.getMaxLOD();
        const float screenSpaceError = vrParams.getSSE();

        DashTree& dashTree = node->getDashTree();

        const VolumeInformation& volInfo = dashTree.getDataSource()->getVolumeInfo();

        const float worldSpacePerVoxel = volInfo.worldSpacePerVoxel;
        const uint32_t volumeDepth = volInfo.rootNode.getDepth();
        _drawRange = _channel->getRange();

        SelectVisibles visitor( dashTree, _frustum,
                                _channel->getPixelViewport().h,
                                screenSpaceError, worldSpacePerVoxel,
                                volumeDepth, minLOD, maxLOD,
                                Range{{ _drawRange.start, _drawRange.end }});

        livre::DFSTraversal traverser;
        traverser.traverse( volInfo.rootNode, visitor,
                            dashTree.getRenderStatus().getFrameID( ));
        window->commit();
        return visitor.getVisibles();
    }

    void updateRegions( const RenderBricks& bricks )
    {
        const Matrix4f& mvpMatrix = _frustum.getMVPMatrix();
        for( const RenderBrick& brick : bricks )
        {
            const Boxf& worldBox = brick.getLODNode().getWorldBox();
            const Vector3f& min = worldBox.getMin();
            const Vector3f& max = worldBox.getMax();
            const Vector3f corners[8] =
            {
                Vector3f( min[0], min[1], min[2] ),
                Vector3f( max[0], min[1], min[2] ),
                Vector3f( min[0], max[1], min[2] ),
                Vector3f( max[0], max[1], min[2] ),
                Vector3f( min[0], min[1], max[2] ),
                Vector3f( max[0], min[1], max[2] ),
                Vector3f( min[0], max[1], max[2] ),
                Vector3f( max[0], max[1], max[2] )
            };

            Vector4f region(  std::numeric_limits< float >::max(),
                              std::numeric_limits< float >::max(),
                             -std::numeric_limits< float >::max(),
                             -std::numeric_limits< float >::max( ));

            for( size_t i = 0; i < 8; ++i )
            {
                const Vector3f corner = mvpMatrix * corners[i];
                region[0] = std::min( corner[0], region[0] );
                region[1] = std::min( corner[1], region[1] );
                region[2] = std::max( corner[0], region[2] );
                region[3] = std::max( corner[1], region[3] );
            }

            // transform ROI from [ -1 -1 1 1 ] to normalized viewport
            const Vector4f normalized( region[0] * .5f + .5f,
                                       region[1] * .5f + .5f,
                                       ( region[2] - region[0] ) * .5f,
                                       ( region[3] - region[1] ) * .5f );

            _channel->declareRegion( eq::Viewport( normalized ));
        }
#ifndef NDEBUG
        _channel->outlineViewport();
#endif
    }

    void freeTexture( const NodeId& nodeId )
    {
        livre::Node* node = static_cast< livre::Node* >( _channel->getNode( ));
        DashTree& dashTree = node->getDashTree();

        dash::NodePtr dashNode = dashTree.getDashNode( nodeId );
        if( !dashNode )
            return;

        DashRenderNode renderNode( dashNode );
        if( renderNode.getLODNode().getRefLevel() != 0 )
            renderNode.setTextureObject( CacheObjectPtr( ));
    }

    void freeTextures()
    {
        for( const auto& cacheObject: _frameInfo.renderNodes )
        {
            const NodeId nodeId(cacheObject->getId( ));
            freeTexture( nodeId );
        }

        for( const NodeId& nodeId: _frameInfo.allNodes )
            freeTexture( nodeId );
    }

    void frameDraw( const eq::uint128_t& )
    {
        livre::Node* node = static_cast< livre::Node* >( _channel->getNode( ));
        const DashRenderStatus& renderStatus = node->getDashTree().getRenderStatus();
        const uint32_t frame = renderStatus.getFrameID();
        if( frame >= INVALID_FRAME )
            return;

        applyCamera();
        setupFrustum();
        _frameInfo = FrameInfo( _frustum, frame );

        const DashRenderNodes& visibles = requestData();

        livre::Window* window = static_cast< livre::Window* >( _channel->getWindow( ));
        const livre::Pipe* pipe = static_cast< const livre::Pipe* >( _channel->getPipe( ));

        const bool isSynchronous =
            pipe->getFrameData()->getVRParameters().getSynchronousMode();

        // #75: only wait for data in synchronous mode
        const bool dashTreeUpdated = window->apply( isSynchronous );

        if( dashTreeUpdated )
        {
            const Frustum& receivedFrustum = renderStatus.getFrustum();

            // If there are multiple channels, this may cause the ping-pong
            // because every channel will try to update the same DashTree in
            // node with their own frustum.
            if( !isSynchronous && receivedFrustum != _frustum )
                _channel->getConfig()->sendEvent( REDRAW );
        }

        const AvailableSetGenerator generateSet( window->getTextureCache( ));

        for( const auto& visible : visibles )
            _frameInfo.allNodes.push_back(visible.getLODNode().getNodeId());
        generateSet.generateRenderingSet( _frameInfo );

        _renderer->update( *pipe->getFrameData( ));
        RenderBricks renderBricks;
        generateRenderBricks( _frameInfo.renderNodes, renderBricks );

        const eq::PixelViewport& vp = _channel->getPixelViewport();
        _renderer->render( _frustum,
                           PixelViewport( vp.x, vp.y, vp.w, vp.h ),
                           renderBricks );
        updateRegions( renderBricks );
        freeTextures();
    }

    void applyCamera()
    {
        const CameraSettings& cameraSettings =
            getFrameData()->getCameraSettings();
        glMultMatrixf( cameraSettings.getModelViewMatrix().array );
    }

    void configInit()
    {
        initializeFrame();
        initializeRenderer();
    }

    void configExit()
    {
        _frameInfo.renderNodes.clear();
        _frame.getFrameData()->flush();
    }

    void addImageListener()
    {
        if( getFrameData()->getFrameSettings().getGrabFrame( ))
            _channel->addResultImageListener( &_frameGrabber );
    }

    void removeImageListener()
    {
        if( getFrameData()->getFrameSettings().getGrabFrame() )
            _channel->removeResultImageListener( &_frameGrabber );
    }

    void frameViewFinish()
    {
        _channel->applyBuffer();
        _channel->applyViewport();

        const FrameSettings& frameSettings = getFrameData()->getFrameSettings();
        if( frameSettings.getStatistics( ))
        {
            _channel->drawStatistics();
            drawCacheStatistics();
        }
    }

    void drawCacheStatistics()
    {
        glLogicOp( GL_XOR );
        glEnable( GL_COLOR_LOGIC_OP );
        glDisable( GL_LIGHTING );
        glDisable( GL_DEPTH_TEST );

        glColor3f( 1.f, 1.f, 1.f );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        _channel->applyScreenFrustum();
        glMatrixMode( GL_MODELVIEW );

        livre::Node* node = static_cast< livre::Node* >( _channel->getNode( ));
        std::ostringstream os;
        const size_t all = _frameInfo.allNodes.size();
        const size_t missing = _frameInfo.notAvailableRenderNodes.size();
        const float done = all > 0 ? float( all - missing ) / float( all ) : 0;
        os << node->getTextureDataCache().getStatistics() << "  "
           << int( 100.f * done + .5f ) << "% loaded" << std::endl;
        float y = 220;
        _drawText( os.str(), y );

        Window* window = static_cast< Window* >( _channel->getWindow( ));
        os.str("");
        os << window->getTextureCache().getStatistics();
        _drawText( os.str(), y );

        ConstDataSourcePtr dataSource = static_cast< livre::Node* >(
            _channel->getNode( ))->getDashTree().getDataSource();
        const VolumeInformation& info = dataSource->getVolumeInfo();
        Vector3f voxelSize = info.boundingBox.getSize() / info.voxels;
        std::string unit = "m";
        if( voxelSize.x() < 0.000001f )
        {
            unit = "um";
            voxelSize *= 1000000;
        }
        if( voxelSize.x() < 0.001f )
        {
            unit = "mm";
            voxelSize *= 1000;
        }

        os.str("");
        os << "Total resolution " << info.voxels << " depth "
           << info.rootNode.getDepth() << std::endl
           << "Block resolution " << info.maximumBlockSize << std::endl
           << unit << "/voxel " << voxelSize;
        _drawText( os.str( ), y );
    }

    void _drawText( std::string text, float& y )
    {
        const eq::util::BitmapFont* font =_channel->getWindow()->getSmallFont();
        for( size_t pos = text.find( '\n' ); pos != std::string::npos;
             pos = text.find( '\n' ))
        {
            glRasterPos3f( 10.f, y, 0.99f );

            font->draw( text.substr( 0, pos ));
            text = text.substr( pos + 1 );
            y -= 16.f;
        }
        // last line
        glRasterPos3f( 10.f, y, 0.99f );
        font->draw( text );
    }

    void frameFinish()
    {
        livre::Node* node = static_cast< livre::Node* >( _channel->getNode( ));
        DashRenderStatus& renderStatus = node->getDashTree().getRenderStatus();
        renderStatus.setFrustum( _frustum );
    }

    void frameReadback( const eq::Frames& frames ) const
    {
        for( eq::Frame* frame : frames ) // Drop depth buffer from output frames
            frame->disableBuffer( eq::Frame::BUFFER_DEPTH );
    }

    void frameAssemble( const eq::Frames& frames )
    {
        eq::PixelViewport coveredPVP;
        eq::Frames dbFrames;

        // Make sure all frames are ready and gather some information on them
        prepareFramesAndSetPvp( frames, dbFrames, coveredPVP );
        coveredPVP.intersect( _channel->getPixelViewport( ));

        if( dbFrames.empty() || !coveredPVP.hasArea( ))
            return;

        if( useDBSelfAssemble( )) // add self to determine ordering
        {
            eq::FrameDataPtr data = _frame.getFrameData();
            _frame.clear( );
            _frame.setOffset( eq::Vector2i( 0, 0 ));
            data->getContext().range = _drawRange;
            data->setPixelViewport( coveredPVP );
            dbFrames.push_back( &_frame );
        }

        orderFrames( dbFrames, computeModelView( ));

        if( useDBSelfAssemble( )) // read back self frame
        {
            if( dbFrames.front() == &_frame ) // OPT: first in framebuffer!
                dbFrames.erase( dbFrames.begin( ));
            else
            {
                _frame.readback( _channel->getObjectManager(),
                                 _channel->getDrawableConfig(),
                                 _channel->getRegions(),
                                 _channel->getContext( ));
                clearViewport( coveredPVP );
                // offset for assembly
                _frame.setOffset( eq::Vector2i( coveredPVP.x, coveredPVP.y ));
            }
        }

        try // blend DB frames in computed order
        {
            eq::Compositor::blendFrames( dbFrames, _channel, 0 );
        }
        catch( const std::exception& e )
        {
            LBWARN << e.what() << std::endl;
        }

        // Update draw range
        for( size_t i = 0; i < dbFrames.size(); ++i )
            _drawRange.merge( dbFrames[i]->getFrameData()->getContext().range );
    }

    bool useDBSelfAssemble() const { return _drawRange != eq::Range::ALL; }

    static bool cmpRangesInc(const eq::Frame* a, const eq::Frame* b )
    {
        return a->getFrameData()->getContext().range.start >
               b->getFrameData()->getContext().range.start;
    }

    void prepareFramesAndSetPvp( const eq::Frames& frames,
                                 eq::Frames& dbFrames,
                                 eq::PixelViewport& coveredPVP )
    {
        for( eq::Frame* frame : frames )
        {
            {
                eq::ChannelStatistics event(
                    eq::Statistic::CHANNEL_FRAME_WAIT_READY, _channel );
                frame->waitReady( );
            }

            const eq::Range& range = frame->getFrameData()->getContext().range;
            if( range == eq::Range::ALL ) // 2D frame, assemble directly
            {
                eq::Compositor::assembleFrame( frame, _channel );
                continue;
            }

            dbFrames.push_back( frame );
            for( const eq::Image* image : frame->getImages( ))
            {
                const eq::PixelViewport imagePVP = image->getPixelViewport() +
                                                   frame->getOffset();
                coveredPVP.merge( imagePVP );
            }
        }
    }

    void orderFrames( eq::Frames& frames, const Matrix4f& modelView )
    {
        LBASSERT( !_channel->useOrtho( ));

        // calculate modelview inversed+transposed matrix
        Matrix3f modelviewITM;
        Matrix4f modelviewIM;
        modelView.inverse( modelviewIM );
        Matrix3f( modelviewIM ).transpose_to( modelviewITM );

        Vector3f norm = modelviewITM * Vector3f( 0.0f, 0.0f, 1.0f );
        norm.normalize();
        std::sort( frames.begin(), frames.end(), cmpRangesInc );

        // cos of angle between normal and vectors from center
        std::vector<double> dotVals;

        // of projection to the middle of slices' boundaries
        for( const eq::Frame* frame : frames )
        {
            const double px = -1.0 +
                            frame->getFrameData()->getContext().range.end * 2.0;
            const Vector4f pS = modelView * Vector4f( 0.0f, 0.0f, px, 1.0f );
            Vector3f pSsub( pS[ 0 ], pS[ 1 ], pS[ 2 ] );
            pSsub.normalize();
            dotVals.push_back( norm.dot( pSsub ));
        }

        const Vector4f pS = modelView * Vector4f( 0.0f, 0.0f, -1.0f, 1.0f );
        eq::Vector3f pSsub( pS[ 0 ], pS[ 1 ], pS[ 2 ] );
        pSsub.normalize();
        dotVals.push_back( norm.dot( pSsub ));

        // check if any slices need to be rendered in reverse order
        size_t minPos = std::numeric_limits< size_t >::max();
        for( size_t i=0; i<dotVals.size()-1; i++ )
            if( dotVals[i] > 0 && dotVals[i+1] > 0 )
                minPos = static_cast< int >( i );

        const size_t nFrames = frames.size();
        minPos++;
        if( minPos < frames.size()-1 )
        {
            eq::Frames framesTmp = frames;

            // copy slices that should be rendered first
            memcpy( &frames[ nFrames-minPos-1 ], &framesTmp[0],
                    (minPos+1)*sizeof( eq::Frame* ) );

            // copy slices that should be rendered last, in reverse order
            for( size_t i=0; i<nFrames-minPos-1; i++ )
                frames[ i ] = framesTmp[ nFrames-i-1 ];
        }
    }

    livre::Channel* const _channel;
    eq::Range _drawRange;
    eq::Frame _frame;
    Frustum _frustum;
    FrameGrabber _frameGrabber;
    FrameInfo _frameInfo;
    std::unique_ptr< RayCastRenderer > _renderer;
};

Channel::Channel( eq::Window* parent )
        : eq::Channel( parent )
        , _impl( new Impl( this ))
{
}

Channel::~Channel()
{    
}

bool Channel::configInit( const eq::uint128_t& initId )
{
    if( !eq::Channel::configInit( initId ) )
        return false;

    _impl->configInit();
    return true;
}

bool Channel::configExit()
{
    _impl->configExit();
    return eq::Channel::configExit();
}

void Channel::frameStart( const eq::uint128_t& frameID,
                          const uint32_t frameNumber )
{
    _impl->_drawRange = eq::Range::ALL;
    eq::Channel::frameStart( frameID, frameNumber );
}

void Channel::frameDraw( const lunchbox::uint128_t& frameId )
{
    eq::Channel::frameDraw( frameId );
    _impl->frameDraw( frameId );
}

void Channel::frameFinish( const eq::uint128_t& frameID, const uint32_t frameNumber )
{
    _impl->frameFinish();
    eq::Channel::frameFinish( frameID, frameNumber );
}

void Channel::frameViewStart( const uint128_t& frameId )
{
    eq::Channel::frameViewStart( frameId );
    _impl->addImageListener();
}

void Channel::frameViewFinish( const eq::uint128_t &frameID )
{
    setupAssemblyState();
    _impl->frameViewFinish();
    resetAssemblyState();
    eq::Channel::frameViewFinish( frameID );
    _impl->removeImageListener();
}

void Channel::frameAssemble( const eq::uint128_t&, const eq::Frames& frames )
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();
    _impl->frameAssemble( frames );
    resetAssemblyState();
}

void Channel::frameReadback( const eq::uint128_t& frameId,
                             const eq::Frames& frames )
{
    _impl->frameReadback( frames );
    eq::Channel::frameReadback( frameId, frames );
}

std::string Channel::getDumpImageFileName() const
{
    const livre::Node* node = static_cast< const livre::Node* >( getNode( ));
    const DashTree& dashTree = node->getDashTree();
    std::stringstream filename;
    filename << std::setw( 5 ) << std::setfill('0')
             << dashTree.getRenderStatus().getFrameID() << ".png";
    return filename.str();
}


}
