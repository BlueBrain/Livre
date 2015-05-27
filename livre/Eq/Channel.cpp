
/* Copyright (c) 2006-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya <maxmah@gmail.com>
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <eq/channel.h>
#include <eq/frameData.h>
#include <eq/compositor.h>
#include <eq/image.h>
#include <eq/gl.h>

#include <livre/Eq/FrameData.h>
#include <livre/Eq/Settings/FrameSettings.h>
#include <livre/Eq/Channel.h>
#include <livre/Eq/Pipe.h>
#include <livre/Eq/Node.h>
#include <livre/Eq/Window.h>
#include <livre/Eq/Error.h>
#include <livre/Eq/Config.h>
#include <livre/Eq/Settings/CameraSettings.h>
#include <livre/Eq/Settings/RenderSettings.h>
#include <livre/Eq/Render/EqContext.h>
#include <livre/Eq/Render/RayCastRenderer.h>

#include <livre/Lib/Configuration/ApplicationParameters.h>
#include <livre/core/DashPipeline/DashConnection.h>
#include <livre/core/DashPipeline/DashProcessorOutput.h>
#include <livre/core/DashPipeline/DashProcessorInput.h>
#include <livre/Lib/Uploaders/TextureUploadProcessor.h>
#include <livre/Lib/Uploaders/DataUploadProcessor.h>
#include <livre/Lib/Render/GRSLowerLOD.h>
#include <livre/Lib/Render/RenderView.h>

#include <livre/core/Render/Frustum.h>
#include <livre/core/Render/GLWidget.h>
#include <livre/core/Dash/DashTree.h>
#include <livre/core/Data/VolumeDataSource.h>

#include <eq/fabric/viewport.h>
#include <eq/fabric/pixelViewport.h>
#include <eq/fabric/range.h>

namespace livre
{

/**
 * The EqRenderView class implements livre \see RenderView for internal use of \see eq::Channel.
 */
class EqRenderView : public RenderView
{
public:
    EqRenderView( Channel *channel )
        : channel_( channel )
    {}

    const Frustum& getFrustum() const final
        { return channel_->initializeLivreFrustum(); }

    Channel* const channel_;
};


/**
 * The EqGlWidget class implements livre \GLWidget for internal use of \see eq::Channel.
 */
class EqGlWidget : public GLWidget
{
public:
    EqGlWidget( Channel *channel )
        : channel_( channel )
    {}

    void setViewport( const View* view LB_UNUSED, Viewporti& viewport ) const
    {
        const eq::PixelViewport& channelPvp = channel_->getPixelViewport();
        viewport.set( Vector2f( channelPvp.x, channelPvp.y ),
                      Vector2f( channelPvp.w, channelPvp.h ) );
    }

    uint32_t getX() const
    {
        return channel_->getPixelViewport().x;
    }

    uint32_t getY() const
    {
        return channel_->getPixelViewport().y;
    }

    uint32_t getWidth() const
    {
        return channel_->getPixelViewport().w;
    }

    uint32_t getHeight() const
    {
        return channel_->getPixelViewport().h;
    }

    Channel* channel_;

};

typedef boost::shared_ptr< EqRenderView > EqRenderViewPtr;

Channel::Channel( eq::Window* parent )
        : eq::Channel( parent )
        , renderViewPtr_( new EqRenderView( this ))
        , glWidgetPtr_( new EqGlWidget( this ))
{
    LBASSERT( parent );
}

Channel::~Channel()
{
}

const Frustum& Channel::initializeLivreFrustum()
{
    ConstCameraSettingsPtr cameraSettings =getFrameData_()->getCameraSettings();
    const Matrix4f& cameraRotation = cameraSettings->getCameraRotation();
    const Matrix4f& modelRotation = cameraSettings->getModelRotation();
    const Vector3f& cameraPosition = cameraSettings->getCameraPosition();

    Matrix4f modelView = modelRotation;
    modelView.set_translation( cameraPosition );
    modelView = cameraRotation * modelView;
    modelView = getHeadTransform() * modelView;

    // Compute cull matrix
    const eq::Frustumf& eqFrustum = eq::Channel::getFrustum();
    const eq::Matrix4f& projection = eqFrustum.compute_matrix();

    currentFrustum_.reset();
    currentFrustum_.initialize( modelView, projection );
    return currentFrustum_;
}

void Channel::initializeFrame_()
{
    setNearFar( 0.1f, 15.0f );

    eq::FrameDataPtr frameData = new eq::FrameData();
    frameData->setBuffers( eq::Frame::BUFFER_COLOR );
    frame_.setFrameData( frameData );
}

void Channel::initializeRenderer_()
{
    const uint32_t nSlices =
        getFrameData_()->getRenderSettings()->getNumberOfSamples();
    ConstVolumeDataSourcePtr dataSource =
        static_cast< Node * >( getNode( ))->getVolumeDataSource();

    // TODO: generalize variable renderer
    renderViewPtr_->setRenderer( RendererPtr(
        new RayCastRenderer( glewGetContext(), nSlices,
                             dataSource->getVolumeInformation().compCount,
                             GL_UNSIGNED_BYTE, GL_LUMINANCE8 )));
}

void Channel::initializeGLContextAndStartUploaders_()
{
    Pipe* pipe = static_cast< Pipe* >( getPipe( ));
    TextureUploadProcessorPtr textureUploadProcessor =
        pipe->getTextureUploadProcessor();

    if( textureUploadProcessor->isRunning( ))
        return;

    Window* wnd = static_cast< Window *>( getWindow( ));

    GLContextPtr currentGlContext( new EqContext( wnd ));
    glWidgetPtr_->setGLContext( currentGlContext );
    textureUploadProcessor->setGLContext( GLContextPtr( new EqContext( wnd )));
    textureUploadProcessor->setGLWidget( glWidgetPtr_ );

    DataUploadProcessorPtr dataUploadProcessor = pipe->getDataUploadProcessor();
    dataUploadProcessor->setGLContext( GLContextPtr( new EqContext( wnd ) ) );
    dataUploadProcessor->setGLWidget( glWidgetPtr_ );
    pipe->startUploadProcessors_();
}

bool Channel::configInit( const eq::uint128_t& initId )
{
    if( !eq::Channel::configInit( initId ) )
        return false;

    initializeFrame_();
    initializeRenderer_();
    initializeGLContextAndStartUploaders_();
    return true;
}

bool Channel::configExit()
{
    Node* node = static_cast< Node *>( getNode( ) );
    DashRenderNode rootNode( node->getDashTree()->getRootNode( ) );
    rootNode.rootSetThreadOp_( TO_EXIT );

    Pipe* pipe = static_cast< Pipe* >( getPipe( ) );
    pipe->stopUploadProcessors_();
    return eq::Channel::configExit();
}

void Channel::frameFinish(const eq::uint128_t& frameID, const uint32_t frameNumber )
{
    Node* node = static_cast< Node *>( getNode( ) );
    DashRenderNode rootNode( node->getDashTree()->getRootNode( ) );
    rootNode.rootSetFrustum_( currentFrustum_ );
    rootNode.rootSetFrameID_( frameNumber );

    Pipe* pipe = static_cast< Pipe *>( getPipe( ) );
    pipe->getProcessor()->getProcessorOutput_()->commit( 0 );

    eq::Channel::frameFinish( frameID, frameNumber );
}

void Channel::frameViewStart( const uint128_t& frameId )
{
    if( getFrameData_( )->getFrameSettings()->getGrabFrame( ) )
        addResultImageListener( &frameGrabber_ );
    eq::Channel::frameViewStart( frameId );
}

void Channel::frameViewFinish( const eq::uint128_t &frameID )
{
    applyBuffer();
    applyViewport();

    FrameSettingsPtr frameSettingsPtr = getFrameData_()->getFrameSettings();
    if( frameSettingsPtr->getStatistics( ) )
        drawStatistics();

    eq::Channel::frameViewFinish( frameID );

    if( frameSettingsPtr->getGrabFrame() )
        removeResultImageListener( &frameGrabber_ );
}

void Channel::frameClear( const eq::uint128_t& frameNumber )
{
    eq::Channel::frameClear( frameNumber );
}

void Channel::clearViewport_( const eq::PixelViewport &pvp )
{
    // clear given area
    glScissor( pvp.x, pvp.y, pvp.w, pvp.h );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // restore assembly state
    const eq::PixelViewport& channelPvp = getPixelViewport( );
    glScissor( 0, 0, channelPvp.w, channelPvp.h );
}

ConstFrameDataPtr Channel::getFrameData_( ) const
{
    const Pipe* pipe = static_cast< const Pipe *>( getPipe( ) );
    return pipe->getFrameData( );
}

// TODO : DB load balancing.
void Channel::frameDraw( const eq::uint128_t& frameId )
{
    eq::Channel::frameDraw( frameId );

    applyCamera_();

    const eq::fabric::Viewport& vp = getViewport( );
    const Viewportf viewport( Vector2f( vp.x, vp.y ), Vector2f( vp.w, vp.h ) );
    renderViewPtr_->setViewport( viewport );

    Viewporti pixelViewport;
    glWidgetPtr_->setViewport( renderViewPtr_.get( ), pixelViewport );

    Node* node = static_cast< Node *>( getNode( ) );
    GRSLowerLOD generateSet( node->getVolumeDataSource()->getVolumeInformation(),
                             node->getDashTree()->getRootNode(),
                             pixelViewport.getHeight(),
                             getFrameData_()->getVRParameters()->screenSpaceError );

    EqRenderViewPtr renderViewPtr = boost::static_pointer_cast< EqRenderView >( renderViewPtr_ );

    renderViewPtr->setParameters( getFrameData_()->getVRParameters(),
                                  getFrameData_()->getEFPParameters( ));

    RayCastRendererPtr renderer = boost::static_pointer_cast< RayCastRenderer >( renderViewPtr->getRenderer( ));

    const Pipe* pipe = static_cast< const Pipe * >( getPipe( ));
    renderer->initTransferFunction( pipe->getFrameData( )->getRenderSettings( )->getTransferFunction( ));

    glWidgetPtr_->renderView( renderViewPtr_.get(), generateSet );
}

void Channel::prepareFramesAndSetPvp_( const eq::Frames& frames,
                                       eq::Frames& dbFrames,
                                       eq::PixelViewport& coveredPVP,
                                       eq::Zoom& zoom )
{
    for( eq::Frames::const_iterator itFrame = frames.begin( ); itFrame != frames.end( ); ++itFrame )
    {
        eq::Frame* frame = *itFrame;

        frame->waitReady( );

        const eq::Range& range = frame->getRange( );
        if( range == eq::Range::ALL ) // 2D frame, assemble directly
        {
            eq::Compositor::assembleFrame( frame, this );
        }
        else
        {
            dbFrames.push_back( frame );
            zoom = frame->getZoom( );

            for( eq::Images::const_iterator itImg = frame->getImages( ).begin( );
                 itImg != frame->getImages( ).end( ); ++itImg )
            {
                const eq::Image *image = *itImg;
                const eq::PixelViewport imagePVP = image->getPixelViewport( ) + frame->getOffset( );
                coveredPVP.merge( imagePVP );
            }
        }
    }
}

void Channel::applyCamera_()
{
    ConstCameraSettingsPtr cameraSettings = getFrameData_( )->getCameraSettings( );

    const Matrix4f& cameraRotation = cameraSettings->getCameraRotation( );
    const Matrix4f& modelRotation = cameraSettings->getModelRotation( );
    const Vector3f& cameraPosition = cameraSettings->getCameraPosition( );

    EQ_GL_CALL( glMultMatrixf( cameraRotation.array ) );
    EQ_GL_CALL( glTranslatef( cameraPosition[ 0 ], cameraPosition[ 1 ], cameraPosition[ 2 ] ) );
    EQ_GL_CALL( glMultMatrixf( modelRotation.array ) );
}

void Channel::composeFrames_( const eq::PixelViewport& coveredPVP,
                              const eq::Zoom& zoom,
                              eq::FrameDataPtr data,
                              eq::Frames& dbFrames )
{
    if( dbFrames.front( ) == &frame_ )
    {
        std::cout << "not erasing main frame" << std::endl;
        dbFrames.erase( dbFrames.begin( ) );
    }
    else if( coveredPVP.hasArea( ) )
    {
        eq::util::ObjectManager& glObjects = getObjectManager( );

        frame_.setOffset( eq::Vector2i( 0, 0 ) );
        frame_.setZoom( zoom );
        data->setPixelViewport( coveredPVP );
        frame_.readback( glObjects, getDrawableConfig( ), getRegions( ) );
        clearViewport_( coveredPVP );

        // offset for assembly
        frame_.setOffset( eq::Vector2i( coveredPVP.x, coveredPVP.y ) );
    }
}

void Channel::frameAssemble( const eq::uint128_t&, const eq::Frames& frames )
{
    const bool composeOnly = ( drawRange_ == eq::Range::ALL );

    applyBuffer( );
    applyViewport( );
    setupAssemblyState( );

    eq::PixelViewport coveredPVP;
    eq::Frames dbFrames;
    eq::Zoom zoom( eq::Zoom::NONE );

    // Make sure all frames are ready and gather some information on them
    prepareFramesAndSetPvp_( frames, dbFrames, coveredPVP, zoom );

    coveredPVP.intersect( getPixelViewport( ) );

    if( dbFrames.empty( ) )
    {
        resetAssemblyState( );
        return;
    }

    // calculate correct frames sequence
    eq::FrameDataPtr data = frame_.getFrameData( );
    if( !composeOnly && coveredPVP.hasArea( ) )
    {
        frame_.clear( );
        data->setRange( drawRange_ );
        dbFrames.push_back( &frame_ );
    }

    // TODO : order frames

    // Update range
    eq::Range newRange( 1.f, 0.f );
    for( size_t i = 0; i < dbFrames.size( ); ++i )
    {
        const eq::Range range = dbFrames[i]->getRange( );
        if( newRange.start > range.start ) newRange.start = range.start;
        if( newRange.end   < range.end   ) newRange.end   = range.end;
    }
    drawRange_ = newRange;

    // check if current frame is in proper position, read back if not
    if( !composeOnly )
    {
        composeFrames_( coveredPVP, zoom, data, dbFrames );
    }

    // blend DB frames in order
    try
    {
        eq::Compositor::assembleFramesSorted( dbFrames, this, 0, true /* blendAlpha */ );
    }
    catch( const std::exception& e )
    {
        LBWARN << e.what( ) << std::endl;
    }

    resetAssemblyState( );
}

void Channel::frameReadback( const eq::uint128_t& frameId,
                             const eq::Frames& frames )
{
    // Drop depth buffer flag from all output frames
    for( eq::FramesCIter i = frames.begin( ); i != frames.end( ); ++i )
    {
        eq::Frame* frame = *i;
        frame->disableBuffer( eq::Frame::BUFFER_DEPTH );
        frame->getFrameData( )->setRange( drawRange_ );
    }

    eq::Channel::frameReadback( frameId, frames );
}

GLWidgetPtr Channel::getGLWidget( )
{
    return glWidgetPtr_;
}

}
