
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
#include <livre/eq/Event.h>
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

#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/render/FrameInfo.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/render/GLWidget.h>
#include <livre/core/render/RenderBrick.h>
#include <livre/core/render/View.h>
#include <livre/core/visitor/RenderNodeVisitor.h>
#include <livre/core/pipeline/Filter.h>
#include <livre/core/pipeline/FilterInput.h>
#include <livre/core/pipeline/FilterOutput.h>
#include <livre/core/pipeline/FunctionFilter.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/pipeline/PortInfo.h>
#include <livre/core/pipeline/InputPort.h>
#include <livre/core/pipeline/Pipeline.h>
#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/SimpleExecutor.h>
#include <livre/core/pipeline/Workers.h>
#include <livre/core/visitor/DFSTraversal.h>

#include <livre/lib/cache/TextureCache.h>
#include <livre/lib/cache/TextureDataCache.h>
#include <livre/lib/cache/TextureObject.h>
#include <livre/lib/pipeline/RenderPipeFilter.h>
#include <livre/lib/pipeline/DataUploadFilter.h>
#include <livre/lib/render/ScreenSpaceLODEvaluator.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>

#include <eq/eq.h>
#include <eq/gl.h>

namespace livre
{

namespace detail
{

/**
 * The EqRenderView class implements livre \see RenderView for internal use of \see eq::Channel.
 */
class EqRenderView : public View
{
public:

    EqRenderView( Channel* channel );
    const Frustum& getFrustum() const final;

private:
    Channel* const _channel;
};


typedef boost::shared_ptr< EqRenderView > EqRenderViewPtr;

/** Implements livre \GLWidget for internal use of eq::Channel. */
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


const float nearPlane = 0.1f;
const float farPlane = 15.0f;

class Channel
{
public:
    explicit Channel( livre::Channel* channel )
              : _channel( channel )
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

        const livre::ConstVolumeDataSourcePtr dataSource = node->getDataSource();

        _renderInput.view.reset( new EqRenderView( this ));
        _renderInput.view->setRenderer( RendererPtr( new RayCastRenderer(
                                    nSamplesPerRay,
                                    nSamplesPerPixel,
                                    dataSource->getVolumeInformation(),
                                    GL_UNSIGNED_BYTE,
                                    GL_LUMINANCE8 )));
    }

    const Frustum& initializeLivreFrustum()
    {
        const eq::Matrix4f& modelView = computeModelView();
        const eq::Frustumf& eqFrustum = _channel->getFrustum();
        const eq::Matrix4f& projection = eqFrustum.compute_matrix();

        _currentFrustum.reset();
        _currentFrustum.initialize( modelView, projection );
        return _currentFrustum;
    }

    eq::Matrix4f computeModelView() const
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

    void notifyRedraw( PipeFilter& pipeFilter )
    {
        pipeFilter.waitForInput();
        livre::Config* config =
               static_cast< livre::Config* >( _channel->getConfig( ));
        config->sendEvent( REDRAW );
    }

    void frameDraw( const eq::uint128_t& )
    {
        livre::Window* window =
               static_cast< livre::Window* >( _channel->getWindow( ));
        const livre::Pipe* pipe =
               static_cast< const livre::Pipe* >( window->getPipe( ));
        const uint32_t frame =
               pipe->getFrameData()->getFrameSettings()->getFrameNumber();

        if( frame >= INVALID_FRAME )
            return;

        applyCamera();
        initializeLivreFrustum();

        _drawRange = _channel->getRange();

        PipelinePtr generateRenderingSetPipe( new Pipeline );
        FilterPtr renderFilter( new RenderPipeFilter( ));
        PipeFilterPtr renderPipeFilter = generateRenderingSetPipe->add( renderFilter );
        renderPipeFilter->setInput( "RenderPipeInput", _renderInput );
        renderPipeFilter->setInput( "Frustum", std::move( _currentFrustum ));
        renderPipeFilter->setInput( "Frame", frame );

        const Vector2f drawRange( _drawRange.start, _drawRange.end );
        renderPipeFilter->setInput( "DataRange", drawRange );
        generateRenderingSetPipe->execute();
    }

    void applyCamera()
    {
        ConstCameraSettingsPtr cameraSettings = getFrameData()->getCameraSettings( );

        const Matrix4f& cameraRotation = cameraSettings->getCameraRotation( );
        const Matrix4f& modelRotation = cameraSettings->getModelRotation( );
        const Vector3f& cameraPosition = cameraSettings->getCameraPosition( );

        EQ_GL_CALL( glMultMatrixf( cameraRotation.array ) );
        EQ_GL_CALL( glTranslatef( cameraPosition[ 0 ], cameraPosition[ 1 ],
                                  cameraPosition[ 2 ] ) );
        EQ_GL_CALL( glMultMatrixf( modelRotation.array ) );
    }

    void configInit()
    {
        initializeFrame();
        initializeRenderer();

        livre::Node* node = static_cast< livre::Node* >( _channel->getNode( ));
        livre::Window* window = static_cast< livre::Window* >( _channel->getWindow( ));
        const livre::Pipe* pipe =
               static_cast< const livre::Pipe* >( window->getPipe( ));

        _renderInput.glWidget.reset( new EqGlWidget( _channel ));

        GLContextPtr glContext( new EqContext( window ));
        _renderInput.glWidget->setGLContext( glContext );

        _renderInput.dataSource = node->getDataSource();
        _renderInput.dataCache = node->getTextureDataCache();
        _renderInput.textureCache = window->getTextureCache();

        PortInfos redrawFilterInputPorts =
                   { PortInfo( "RenderPipeInput", RenderPipeFilter::RenderPipeInput()) };
        PortInfos redrawFilterOutputPorts =
                   { PortInfo( "RenderPipeInput", RenderPipeFilter::RenderPipeInput()) };

        FilterPtr redrawFilter( new FunctionFilter( boost::bind( &Channel::notifyRedraw,
                                                                   this,
                                                                   _1),
                                                                 redrawFilterInputPorts,
                                                                 redrawFilterOutputPorts ));

        ConstVolumeRendererParametersPtr vrParams =
                                   pipe->getFrameData()->getVRParameters();
        _renderInput.redrawFilter = redrawFilter;
        _renderInput.computeExecutor = window->getComputeExecutor();
        _renderInput.uploadExecutor = window->getUploadExecutor();
        _renderInput.isSynchronous = vrParams->synchronousMode;
        _renderInput.maxLOD = vrParams->maxLOD;
        _renderInput.minLOD = vrParams->minLOD;
        _renderInput.screenSpaceError = vrParams->screenSpaceError;
    }

    void configExit()
    {
        _frame.getFrameData()->flush();
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

        Window* window = static_cast< Window* >( _channel->getWindow( ));
        std::ostringstream os;
        os.str("");
        os << window->getTextureCache()->getStatistics();
        float y = 220;
        _drawText( os.str(), y );

        ConstVolumeDataSourcePtr dataSource = static_cast< livre::Node* >(
            _channel->getNode( ))->getDataSource();
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
            data->setRange( _drawRange );
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
                                 _channel->getRegions( ));
                clearViewport( coveredPVP );
                // offset for assembly
                _frame.setOffset( eq::Vector2i( coveredPVP.x, coveredPVP.y ));
            }
        }

        LBINFO << "Frame order: ";
        for( const eq::Frame* frame : dbFrames )
            LBINFO << frame->getName() <<  " "
                   << frame->getFrameData()->getRange() << " : ";
        LBINFO << std::endl;

        try // blend DB frames in computed order
        {
            eq::Compositor::assembleFramesSorted( dbFrames, _channel, 0,
                                                  true /* blendAlpha */ );
        }
        catch( const std::exception& e )
        {
            LBWARN << e.what() << std::endl;
        }

        // Update draw range
        for( size_t i = 0; i < dbFrames.size(); ++i )
            _drawRange.merge( dbFrames[i]->getRange( ));
    }

    bool useDBSelfAssemble() const { return _drawRange != eq::Range::ALL; }

    static bool cmpRangesInc(const eq::Frame* a, const eq::Frame* b )
        { return a->getRange().start > b->getRange().start; }

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

            const eq::Range& range = frame->getRange();
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
            const double px = -1.0 + frame->getRange().end*2.0;
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

    std::string getDumpImageFileName() const
    {
        std::stringstream filename;

        const livre::Window* window =
               static_cast< livre::Window* >( _channel->getWindow( ));
        const livre::Pipe* pipe =
               static_cast< const livre::Pipe* >( window->getPipe( ));
        const uint32_t frame =
               pipe->getFrameData()->getFrameSettings()->getFrameNumber();

        filename << std::setw( 5 ) << std::setfill('0')
                 << frame << ".png";
        return filename.str();
    }


    livre::Channel* const _channel;
    eq::Range _drawRange;
    eq::Frame _frame;
    Frustum _currentFrustum;
    ViewPtr _renderViewPtr;
    GLWidgetPtr _glWidgetPtr;
    FrameGrabber _frameGrabber;
    RenderPipeFilter::RenderPipeInput _renderInput;
};

EqRenderView::EqRenderView(Channel* channel)
    : _channel( channel )
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
    return _impl->getDumpImageFileName();
}


}
