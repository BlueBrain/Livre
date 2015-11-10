
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
#include <livre/lib/render/RenderView.h>
#include <livre/lib/render/ScreenSpaceLODEvaluator.h>
#include <livre/lib/visitor/DFSTraversal.h>

#include <livre/core/dash/DashRenderStatus.h>
#include <livre/core/dash/DashTree.h>
#include <livre/core/dashpipeline/DashProcessorInput.h>
#include <livre/core/dashpipeline/DashProcessorOutput.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/render/FrameInfo.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/GLWidget.h>
#include <livre/core/render/RenderBrick.h>
#include <livre/core/visitor/RenderNodeVisitor.h>

#include <eq/eq.h>
#include <eq/gl.h>

namespace livre
{

namespace detail
{

/**
 * The EqRenderView class implements livre \see RenderView for internal use of \see eq::Channel.
 */
class EqRenderView : public RenderView
{
public:

    EqRenderView( Channel* channel, ConstDashTreePtr dashTree );
    const Frustum& getFrustum() const final;

private:
    Channel* const _channel;
};


typedef boost::shared_ptr< EqRenderView > EqRenderViewPtr;

/**
 * The EqGlWidget class implements livre \GLWidget for internal use of \see eq::Channel.
 */
class EqGlWidget : public GLWidget
{
public:
    explicit EqGlWidget( livre::Channel* channel )
        : _channel( channel )
    {}

    Viewport getViewport( const View& ) const final
    {
        const eq::PixelViewport& channelPvp = _channel->getPixelViewport();
        return Viewport( channelPvp.x, channelPvp.y,
                         channelPvp.w, channelPvp.h );
    }

    uint32_t getX() const
    {
        return _channel->getPixelViewport().x;
    }

    uint32_t getY() const
    {
        return _channel->getPixelViewport().y;
    }

    uint32_t getWidth() const
    {
        return _channel->getPixelViewport().w;
    }

    uint32_t getHeight() const
    {
        return _channel->getPixelViewport().h;
    }

    livre::Channel* _channel;
};

class SelectVisibles : public livre::RenderNodeVisitor
{
public:

    SelectVisibles( DashTreePtr dashTree,
                    const Frustum& frustum,
                    const uint32_t windowHeight,
                    const float screenSpaceError,
                    const float worldSpacePerVoxel,
                    const uint32_t volumeDepth,
                    const uint32_t minDepth,
                    const uint32_t maxDepth );

    void visit( DashRenderNode& renderNode, VisitState& state ) final;

    const ScreenSpaceLODEvaluator _lodEvaluator;
    const Frustum& _frustum;
    const uint32_t _volumeDepth;
};

SelectVisibles::SelectVisibles( DashTreePtr dashTree,
                                const Frustum& frustum,
                                const uint32_t windowHeight,
                                const float screenSpaceError,
                                const float worldSpacePerVoxel,
                                const uint32_t volumeDepth,
                                const uint32_t minLOD,
                                const uint32_t maxLOD )
    : RenderNodeVisitor( dashTree )
    , _lodEvaluator( windowHeight,
                     screenSpaceError,
                     worldSpacePerVoxel,
                     minLOD,
                     maxLOD )
    , _frustum( frustum )
    , _volumeDepth( volumeDepth )
{}

void SelectVisibles::visit( DashRenderNode& renderNode, VisitState& state )
{
    const LODNode& lodNode = renderNode.getLODNode();
    if( !lodNode.isValid( ))
        return;

    const Boxf& worldBox = lodNode.getWorldBox();
    const bool isInFrustum = _frustum.boxInFrustum( worldBox );
    renderNode.setInFrustum( isInFrustum );
    if( !isInFrustum )
    {
        state.setVisitChild( false );
        return;
    }

    const Plane& nearPlane = _frustum.getWPlane( PL_NEAR );
    Vector3f vmin, vmax;
    nearPlane.getNearFarPoints( worldBox, vmin, vmax );

    const uint32_t lod =
        _lodEvaluator.getLODForPoint( _frustum, _volumeDepth, vmin );

    const bool isLODVisible = (lod <= lodNode.getNodeId().getLevel( ));
    renderNode.setVisible( isLODVisible );
    state.setVisitChild( !isLODVisible );
}

const float nearPlane = 0.1f;
const float farPlane = 15.0f;

class Channel
{
public:
    explicit Channel( livre::Channel* channel )
          : _channel( channel )
          , _glWidgetPtr( new EqGlWidget( channel ))
          , _frameInfo( _currentFrustum )
    {}

    void initializeFrame()
    {
        _channel->setNearFar( nearPlane, farPlane );

        eq::FrameDataPtr frameData = new eq::FrameData();
        frameData->setBuffers( eq::Frame::BUFFER_COLOR );
        _frame.setFrameData( frameData );
    }

    ConstFrameDataPtr getFrameData() const
    {
        const livre::Pipe* pipe = static_cast< const livre::Pipe* >( _channel->getPipe( ));
        return pipe->getFrameData();
    }

    void initializeRenderer()
    {
        const uint32_t nSamplesPerRay =
            getFrameData()->getVRParameters()->samplesPerRay;

        const uint32_t nSamplesPerPixel =
            getFrameData()->getVRParameters()->samplesPerPixel;

        const livre::Node* node =
                static_cast< livre::Node* >( _channel->getNode( ));

        ConstDashTreePtr dashTree = node->getDashTree();

        ConstVolumeDataSourcePtr dataSource = dashTree->getDataSource();

        _renderViewPtr.reset( new EqRenderView( this, dashTree ));

        RendererPtr renderer( new RayCastRenderer(
                                  nSamplesPerRay,
                                  nSamplesPerPixel,
                                  dataSource->getVolumeInformation(),
                                  GL_UNSIGNED_BYTE,
                                  GL_LUMINANCE8 ));

        _renderViewPtr->setRenderer( renderer);
    }

    const Frustum& initializeLivreFrustum()
    {
        ConstCameraSettingsPtr cameraSettings =
                getFrameData()->getCameraSettings();
        const Matrix4f& cameraRotation = cameraSettings->getCameraRotation();
        const Matrix4f& modelRotation = cameraSettings->getModelRotation();
        const Vector3f& cameraPosition = cameraSettings->getCameraPosition();

        Matrix4f modelView = modelRotation;
        modelView.set_translation( cameraPosition );
        modelView = cameraRotation * modelView;
        modelView = _channel->getHeadTransform() * modelView;

        // Compute cull matrix
        const eq::Frustumf& eqFrustum = _channel->getFrustum();
        const eq::Matrix4f& projection = eqFrustum.compute_matrix();

        _currentFrustum.reset();
        _currentFrustum.initialize( modelView, projection );
        return _currentFrustum;
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
        BOOST_FOREACH( const ConstCacheObjectPtr& cacheObject, renderNodes )
        {
            const ConstTextureObjectPtr texture =
                boost::static_pointer_cast< const TextureObject >( cacheObject );

            RenderBrickPtr renderBrick( new RenderBrick( texture->getLODNode(),
                                                         texture->getTextureState( )));
            renderBricks.push_back( renderBrick );
        }
    }

    void requestData()
    {
        const eq::PixelViewport& channelPvp = _channel->getPixelViewport();
        const Viewport pixelViewport( channelPvp.x, channelPvp.y,
                                      channelPvp.w, channelPvp.h );

        livre::Node* node = static_cast< livre::Node* >( _channel->getNode( ));
        livre::Window* window = static_cast< livre::Window* >( _channel->getWindow( ));
        livre::Pipe* pipe = static_cast< livre::Pipe* >( window->getPipe( ));

        ConstVolumeRendererParametersPtr vrParams = pipe->getFrameData()->getVRParameters();
        const uint32_t minLOD = vrParams->minLOD;
        const uint32_t maxLOD = vrParams->maxLOD;
        const float screenSpaceError = vrParams->screenSpaceError;

        DashTreePtr dashTree = node->getDashTree();

        const VolumeInformation& volInfo = dashTree->getDataSource()->getVolumeInformation();

        const float worldSpacePerVoxel = volInfo.worldSpacePerVoxel;
        const uint32_t volumeDepth = volInfo.rootNode.getDepth();

        SelectVisibles visitor( dashTree,
                                _currentFrustum,
                                pixelViewport[3],
                                screenSpaceError,
                                worldSpacePerVoxel,
                                volumeDepth,
                                minLOD,
                                maxLOD );

        livre::DFSTraversal traverser;
        traverser.traverse( volInfo.rootNode,
                            visitor, dashTree->getRenderStatus().getFrameID( ));

        window->commit();
    }

    void frameDraw( const eq::uint128_t& )
    {
        applyCamera();

        livre::Node* node = static_cast< livre::Node* >( _channel->getNode( ));
        DashTreePtr dashTree = node->getDashTree();
        const uint32_t frame = dashTree->getRenderStatus().getFrameID();
        const VolumeInformation& volInfo =
                dashTree->getDataSource()->getVolumeInformation();
        const Vector2ui& frameRange = volInfo.getFrameRange();
        if( frame < frameRange[0] || frame >= frameRange[1] )
            return;

        initializeLivreFrustum();
        requestData();

        const eq::fabric::Viewport& vp = _channel->getViewport( );
        const Viewport viewport( vp.x, vp.y, vp.w, vp.h );
        _renderViewPtr->setViewport( viewport );

        livre::Window* window = static_cast< livre::Window* >( _channel->getWindow( ));
        AvailableSetGenerator generateSet( node->getDashTree( ),
                                           window->getTextureCache( ));

        _frameInfo.clear();
        generateSet.generateRenderingSet( _currentFrustum, _frameInfo );

        const livre::Pipe* pipe = static_cast< const livre::Pipe* >( _channel->getPipe( ));

        // #75: only wait for data in synchronous mode
        bool dashTreeUpdated = false;

        if( pipe->getFrameData()->getVRParameters()->synchronousMode )
        {
            if( _frameInfo.notAvailableRenderNodes.empty( ))
                dashTreeUpdated = window->apply( false ); // Do not block if rendering nodes are present
            else
                dashTreeUpdated = window->apply( true ); // Block until rendering nodes arrive
        }
        else
            dashTreeUpdated = window->apply( false ); // Never block

        if( dashTreeUpdated )
        {
            _frameInfo.clear();
            generateSet.generateRenderingSet( _currentFrustum, _frameInfo );
        }

        EqRenderViewPtr renderViewPtr =
                boost::static_pointer_cast< EqRenderView >( _renderViewPtr );
        RayCastRendererPtr renderer =
                boost::static_pointer_cast< RayCastRenderer >(
                    renderViewPtr->getRenderer( ));

        renderer->initTransferFunction(
            pipe->getFrameData()->getRenderSettings()->getTransferFunction( ));

        RenderBricks renderBricks;
        generateRenderBricks( _frameInfo.renderNodes, renderBricks );
        renderViewPtr->render( _frameInfo, renderBricks, *_glWidgetPtr );
    }

    void prepareFramesAndSetPvp( const eq::Frames& frames,
                                 eq::Frames& dbFrames,
                                 eq::PixelViewport& coveredPVP,
                                 eq::Zoom& zoom )
    {
        BOOST_FOREACH( eq::Frame* frame, frames )
        {
            frame->waitReady( );

            const eq::Range& range = frame->getRange();
            if( range == eq::Range::ALL ) // 2D frame, assemble directly
            {
                eq::Compositor::assembleFrame( frame, _channel );
            }
            else
            {
                dbFrames.push_back( frame );
                zoom = frame->getZoom( );

                BOOST_FOREACH( const eq::Image* image, frame->getImages( ))
                {
                    const eq::PixelViewport imagePVP = image->getPixelViewport( ) +
                            frame->getOffset( );
                    coveredPVP.merge( imagePVP );
                }
            }
        }
    }

    void applyCamera()
    {
        ConstCameraSettingsPtr cameraSettings = getFrameData()->getCameraSettings( );

        const Matrix4f& cameraRotation = cameraSettings->getCameraRotation( );
        const Matrix4f& modelRotation = cameraSettings->getModelRotation( );
        const Vector3f& cameraPosition = cameraSettings->getCameraPosition( );

        EQ_GL_CALL( glMultMatrixf( cameraRotation.array ) );
        EQ_GL_CALL( glTranslatef( cameraPosition[ 0 ], cameraPosition[ 1 ], cameraPosition[ 2 ] ) );
        EQ_GL_CALL( glMultMatrixf( modelRotation.array ) );
    }

    void composeFrames( const eq::PixelViewport& coveredPVP,
                        const eq::Zoom& zoom,
                        eq::FrameDataPtr data,
                        eq::Frames& dbFrames )
    {
        if( dbFrames.front( ) == &_frame )
        {
            LBWARN << "not erasing main frame" << std::endl;
            dbFrames.erase( dbFrames.begin( ) );
        }
        else if( coveredPVP.hasArea( ) )
        {
            eq::util::ObjectManager& glObjects = _channel->getObjectManager( );

            _frame.setOffset( eq::Vector2i( 0, 0 ) );
            _frame.setZoom( zoom );
            data->setPixelViewport( coveredPVP );
            _frame.readback( glObjects,
                             _channel->getDrawableConfig( ),
                             _channel->getRegions( ));
            clearViewport( coveredPVP );

            // offset for assembly
            _frame.setOffset( eq::Vector2i( coveredPVP.x, coveredPVP.y ) );
        }
    }

    void configInit()
    {
        initializeFrame();
        initializeRenderer();

        Window* window = static_cast< Window* >( _channel->getWindow( ));
        _glWidgetPtr->setGLContext( GLContextPtr( new EqContext( window )));
    }

    void configExit()
    {
        _renderViewPtr.reset();
    }

    void addImageListener()
    {
        if( getFrameData( )->getFrameSettings()->getGrabFrame( ))
            _channel->addResultImageListener( &_frameGrabber );
    }

    void removeImageListener()
    {
        if( getFrameData()->getFrameSettings()->getGrabFrame() )
            _channel->removeResultImageListener( &_frameGrabber );
    }

    void frameViewFinish()
    {
        _channel->applyBuffer();
        _channel->applyViewport();

        FrameSettingsPtr frameSettingsPtr = getFrameData()->getFrameSettings();
        if( frameSettingsPtr->getStatistics( ))
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

        ConstVolumeDataSourcePtr dataSource = static_cast< livre::Node* >(
            _channel->getNode( ))->getDashTree()->getDataSource();
        const VolumeInformation& info = dataSource->getVolumeInformation();
        Vector3f voxelSize = info.boundingBox.getDimension() / info.voxels;
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
        os << "Total resolution " << info.voxels  << " depth "
           << lunchbox::getIndexOfLastBit( info.voxels.x() /
                                           info.maximumBlockSize.x( ))
           << std::endl
           << "Block resolution " << info.maximumBlockSize << std::endl
           << unit << "/voxel " << voxelSize;
        _drawText( os.str( ), y );
    }

    void _drawText( std::string text, float& y )
    {
        const eq::util::BitmapFont* font = _channel->getWindow()->getSmallFont();
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
        DashRenderStatus& renderStatus = node->getDashTree()->getRenderStatus();
        renderStatus.setFrustum( _currentFrustum );
    }

    void frameReadback( const eq::Frames& frames ) const
    {
        // Drop depth buffer flag from all output frames
        BOOST_FOREACH( eq::Frame* frame, frames )
        {
            frame->disableBuffer( eq::Frame::BUFFER_DEPTH );
            frame->getFrameData()->setRange( _drawRange );
        }
    }

    bool composeOnly() const
    {
        return _drawRange == eq::Range::ALL;
    }

    void frameAssemble( const eq::Frames& frames )
    {
        eq::PixelViewport coveredPVP;
        eq::Frames dbFrames;
        eq::Zoom zoom( eq::Zoom::NONE );

        // Make sure all frames are ready and gather some information on them
        prepareFramesAndSetPvp( frames, dbFrames, coveredPVP, zoom );

        if( dbFrames.empty( ))
            return;

        coveredPVP.intersect( _channel->getPixelViewport( ));

        // calculate correct frames sequence
        eq::FrameDataPtr data = _frame.getFrameData();

        if( !composeOnly() && coveredPVP.hasArea( ))
        {
            _frame.clear( );
            data->setRange( _drawRange );
            dbFrames.push_back( &_frame );
        }

        // Update range
        eq::Range newRange( 1.f, 0.f );
        for( size_t i = 0; i < dbFrames.size(); ++i )
        {
            const eq::Range& range = dbFrames[i]->getRange( );
            if( newRange.start > range.start )
                newRange.start = range.start;
            if( newRange.end   < range.end   )
                newRange.end   = range.end;
        }

        _drawRange = newRange;

        // check if current frame is in proper position, read back if not
        if( !composeOnly( ))
            composeFrames( coveredPVP, zoom, data, dbFrames );

        // blend DB frames in order
        try
        {
            eq::Compositor::assembleFramesSorted( dbFrames, _channel, 0, true /* blendAlpha */ );
        }
        catch( const std::exception& e )
        {
            LBWARN << e.what( ) << std::endl;
        }
    }

    livre::Channel* const _channel;
    eq::Range _drawRange;
    eq::Frame _frame;
    Frustum _currentFrustum;
    ViewPtr _renderViewPtr;
    GLWidgetPtr _glWidgetPtr;
    FrameGrabber _frameGrabber;
    FrameInfo _frameInfo;
};

EqRenderView::EqRenderView( Channel* channel,
                            ConstDashTreePtr dashTree )
    : RenderView( dashTree )
    , _channel( channel )
{}

const Frustum& EqRenderView::getFrustum() const
{
    return _channel->initializeLivreFrustum();
}

}

Channel::Channel( eq::Window* parent )
        : eq::Channel( parent )
        , _impl( new detail::Channel( this ))
{
}

Channel::~Channel()
{
    delete _impl;
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
    eq::Channel::frameReadback( frameId, frames );
    _impl->frameReadback( frames );
}

std::string Channel::getDumpImageFileName() const
{
    const livre::Node* node = static_cast< const livre::Node* >( getNode( ));
    ConstDashTreePtr dashTree = node->getDashTree();
    std::stringstream filename;
    filename << std::setw( 5 ) << std::setfill('0')
             << dashTree->getRenderStatus().getFrameID() << ".png";
    return filename.str();
}


}
