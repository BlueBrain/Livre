
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
#include <livre/Lib/Render/AvailableSetGenerator.h>
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

namespace detail
{

/**
 * The EqRenderView class implements livre \see RenderView for internal use of \see eq::Channel.
 */
class EqRenderView : public RenderView
{
public:
    EqRenderView( Channel* channel )
        : _channel( channel )
    {}

    const Frustum& getFrustum() const final;

    Channel* const _channel;
};


typedef boost::shared_ptr< EqRenderView > EqRenderViewPtr;

/**
 * The EqGlWidget class implements livre \GLWidget for internal use of \see eq::Channel.
 */
class EqGlWidget : public GLWidget
{
public:
    EqGlWidget( livre::Channel* channel )
        : _channel( channel )
    {}

    void setViewport( const View*,
                      Viewporti& viewport ) const final
    {
        const eq::PixelViewport& channelPvp = _channel->getPixelViewport();
        viewport.set( Vector2f( channelPvp.x, channelPvp.y ),
                      Vector2f( channelPvp.w, channelPvp.h ) );
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

const float nearPlane = 0.1f;
const float farPlane = 15.0f;

class Channel
{
public:

    Channel( livre::Channel *channel )
          : _renderViewPtr( new EqRenderView( this ))
          , _glWidgetPtr( new EqGlWidget( channel ))
          , _channel( channel )
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
        const uint32_t nSlices =
            getFrameData()->getRenderSettings()->getNumberOfSamples();
        ConstVolumeDataSourcePtr dataSource =
            static_cast< livre::Node* >( _channel->getNode( ))->getDashTree()->getDataSource();

        _renderViewPtr->setRenderer( RendererPtr(
            new RayCastRenderer( _channel->glewGetContext(), nSlices,
                                 dataSource->getVolumeInformation().compCount,
                                 GL_UNSIGNED_BYTE, GL_LUMINANCE8 )));
    }

    void initializeGLContextAndStartUploaders()
    {
        livre::Pipe* pipe = static_cast< livre::Pipe* >( _channel->getPipe( ));
        Window* wnd = static_cast< Window* >( _channel->getWindow( ));

        GLContextPtr currentGlContext( new EqContext( wnd ));
        _glWidgetPtr->setGLContext( currentGlContext );

        TextureUploadProcessorPtr textureUploadProcessor =
            pipe->getTextureUploadProcessor();
        textureUploadProcessor->setGLContext( GLContextPtr( new EqContext( wnd )));
        textureUploadProcessor->setGLWidget( _glWidgetPtr );

        DataUploadProcessorPtr dataUploadProcessor = pipe->getDataUploadProcessor();
        dataUploadProcessor->setGLContext( GLContextPtr( new EqContext( wnd ) ) );
        dataUploadProcessor->setGLWidget( _glWidgetPtr );
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

    // TODO : DB load balancing.
    void frameDraw( const eq::uint128_t& )
    {
        applyCamera();

        const eq::fabric::Viewport& vp = _channel->getViewport( );
        const Viewportf viewport( Vector2f( vp.x, vp.y ),
                                  Vector2f( vp.w, vp.h ));
        _renderViewPtr->setViewport( viewport );

        Viewporti pixelViewport;
        _glWidgetPtr->setViewport( _renderViewPtr.get( ), pixelViewport );

        livre::Node* node = static_cast< livre::Node* >( _channel->getNode( ));
        AvailableSetGenerator generateSet( node->getDashTree(),
                                           pixelViewport.getHeight(),
                                           getFrameData()->getVRParameters()->screenSpaceError );

        EqRenderViewPtr renderViewPtr =
                boost::static_pointer_cast< EqRenderView >( _renderViewPtr );

        renderViewPtr->setParameters( getFrameData()->getVRParameters(),
                                      getFrameData()->getEFPParameters());

        RayCastRendererPtr renderer =
                boost::static_pointer_cast< RayCastRenderer >(
                    renderViewPtr->getRenderer( ));

        const livre::Pipe* pipe = static_cast< const livre::Pipe* >( _channel->getPipe());
        renderer->initTransferFunction(
                    pipe->getFrameData()->getRenderSettings()->getTransferFunction( ));

        _glWidgetPtr->renderView( _renderViewPtr.get(), generateSet );
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

                BOOST_FOREACH( const eq::Image *image, frame->getImages() )
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
        ConstCameraSettingsPtr cameraSettings = getFrameData( )->getCameraSettings( );

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
        initializeGLContextAndStartUploaders();
    }

    void configExit()
    {
        livre::Node* node = static_cast< livre::Node* >( _channel->getNode( ));
        node->getDashTree()->getRenderStatus().setThreadOp( TO_EXIT );
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
            _channel->drawStatistics();
    }

    void frameFinish( const uint32_t frameNumber )
    {
        livre::Node* node = static_cast< livre::Node* >( _channel->getNode( ));
        DashRenderStatus& renderStatus = node->getDashTree()->getRenderStatus();
        renderStatus.setFrustum( _currentFrustum );
        renderStatus.setFrameID( frameNumber );

        livre::Pipe* pipe = static_cast< livre::Pipe* >( _channel->getPipe( ));
        pipe->getProcessor()->getProcessorOutput_()->commit( 0 );
    }

    void frameReadback( const eq::Frames& frames )
    {
        // Drop depth buffer flag from all output frames
        BOOST_FOREACH( eq::Frame* frame, frames )
        {
            frame->disableBuffer( eq::Frame::BUFFER_DEPTH );
            frame->getFrameData( )->setRange( _drawRange );
        }
    }

    bool composeOnly() const
    {
        return ( _drawRange == eq::Range::ALL );
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
        {
            composeFrames( coveredPVP, zoom, data, dbFrames );
        }

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

    eq::Range _drawRange;
    eq::Frame _frame;
    Frustum _currentFrustum;
    ViewPtr _renderViewPtr;
    GLWidgetPtr _glWidgetPtr;
    FrameGrabber _frameGrabber;
    livre::Channel* _channel;
};

const Frustum& EqRenderView::getFrustum() const { return _channel->initializeLivreFrustum(); }

}

Channel::Channel( eq::Window* parent )
        : eq::Channel( parent )
        , _impl( new detail::Channel( this ))

{
     LBASSERT( parent );
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
    _impl->frameDraw( frameId );
    eq::Channel::frameDraw( frameId );
}

void Channel::frameFinish( const eq::uint128_t& frameID, const uint32_t frameNumber )
{
    _impl->frameFinish( frameNumber );
    eq::Channel::frameFinish( frameID, frameNumber );
}

void Channel::frameViewStart( const uint128_t& frameId )
{
    _impl->addImageListener();
    eq::Channel::frameViewStart( frameId );
}

void Channel::frameViewFinish( const eq::uint128_t &frameID )
{
    _impl->frameViewFinish();
    eq::Channel::frameViewFinish( frameID );
    _impl->removeImageListener();
}

void Channel::frameAssemble( const eq::uint128_t&, const eq::Frames& frames )
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();
    _impl->frameAssemble( frames );
    resetAssemblyState( );
}

void Channel::frameReadback( const eq::uint128_t& frameId,
                             const eq::Frames& frames )
{
    _impl->frameReadback( frames );
    eq::Channel::frameReadback( frameId, frames );
}

}
