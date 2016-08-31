
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <livre/eq/settings/VolumeSettings.h>
#include <livre/eq/Window.h>

#include <livre/lib/cache/TextureObject.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>
#include <livre/lib/pipeline/RenderPipeline.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/cache/CacheStatistics.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/data/Histogram.h>
#include <livre/core/render/FrameInfo.h>
#include <livre/core/render/Frustum.h>
#include <livre/core/visitor/DFSTraversal.h>

#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/pipeline/FutureMap.h>
#include <livre/core/pipeline/Filter.h>

#ifdef LIVRE_USE_ZEROEQ
#  include <zeroeq/publisher.h>
#endif
#include <lexis/data/Progress.h>
#include <eq/eq.h>
#include <eq/gl.h>

namespace livre
{
const float nearPlane = 0.1f;
const float farPlane = 15.0f;

struct RedrawFilter : public Filter
{
    explicit RedrawFilter( Channel* channel )
        : _channel( channel )
    {}

    ~RedrawFilter() {}
    void execute( const FutureMap& input,
                  PromiseMap& ) const final
    {
        waitForAny( input.getFutures( ));

        const bool isRenderingDone =  input.get< bool >( "RenderingDone" )[ 0 ];
        if( !isRenderingDone )
            _channel->getConfig()->sendEvent( REDRAW );
    }

    DataInfos getInputDataInfos() const final
    {
        return
        {
            { "CacheObjects", getType< ConstCacheObjects >() },
            { "RenderingDone", getType< bool >() }
        };
    }

    Channel* _channel;
};


struct SendHistogramFilter : public Filter
{
    explicit SendHistogramFilter( Channel* channel )
        : _channel( channel )
    {}

    ~SendHistogramFilter() {}
    void execute( const FutureMap& input,
                  PromiseMap& ) const final
    {
        const UniqueFutureMap uniqueInputs( input.getFutures( ));
        const auto& histogram = uniqueInputs.get< Histogram >( "Histogram" );
        const auto& viewport = uniqueInputs.get< Viewport >( "RelativeViewport" );
        const auto frameCounter =  uniqueInputs.get< uint32_t >( "Id" );
        const_cast< eq::Config *>( _channel ->getConfig())->sendEvent( HISTOGRAM_DATA )
                << histogram
                << ( viewport[ 2 ] * viewport[ 3 ]) // area of the viewport ( w * h )
                << frameCounter;
    }

    DataInfos getInputDataInfos() const final
    {
        return
        {
            { "Histogram", getType< Histogram >() },
            { "RelativeViewport", getType< Viewport >() },
            { "Id", getType< uint32_t >() }
        };
    }

    Channel* _channel;
};

struct EqRaycastRenderer : public RayCastRenderer
{
    EqRaycastRenderer( Channel::Impl& channel,
                       const DataSource& dataSource,
                       const Cache& textureCache,
                       uint32_t samplesPerRay,
                       uint32_t samplesPerPixel )
        : RayCastRenderer( dataSource, textureCache, samplesPerRay, samplesPerPixel )
        , _channel( channel )
    {}

    void _onFrameStart( const Frustum& frustum,
                        const ClipPlanes& planes,
                        const PixelViewport& view,
                        const NodeIds& renderBricks ) final;

    Channel::Impl& _channel;
};


struct Channel::Impl
{
public:
    explicit Impl( Channel* channel )
          : _channel( channel )
          , _progress( "Loading bricks", 0 )
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
        const uint32_t nSamplesPerRay = getFrameData()->getVRParameters().getSamplesPerRay();
        const uint32_t nSamplesPerPixel = getFrameData()->getVRParameters().getSamplesPerPixel();

        const Window* window = static_cast< const Window* >( _channel->getWindow( ));
        const Node* node = static_cast< const Node* >( _channel->getNode( ));
        _renderer.reset( new EqRaycastRenderer( *this,
                                                node->getDataSource(),
                                                window->getTextureCache(),
                                                nSamplesPerRay,
                                                nSamplesPerPixel ));
    }

    Frustum setupFrustum() const
    {
        const eq::Matrix4f& modelView = computeModelView();
        const eq::Frustumf& eqFrustum = _channel->getFrustum();
        const eq::Matrix4f& projection = eqFrustum.computePerspectiveMatrix();
        return Frustum( modelView, projection );
    }

    Matrix4f computeModelView() const
    {
        const CameraSettings& cameraSettings =
            getFrameData()->getCameraSettings();
        return _channel->getHeadTransform() * cameraSettings.getModelViewMatrix();
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

    void updateRegions( const NodeIds& renderBricks,
                        const Frustum& frustum )
    {
        const Matrix4f& mvpMatrix = frustum.getMVPMatrix();

        livre::Node* node =
                static_cast< livre::Node* >( _channel->getNode( ));
        const DataSource& dataSource = node->getDataSource();

        for( const NodeId& nodeId : renderBricks )
        {
            const LODNode& lodNode = dataSource.getNode( nodeId );
            const Boxf& worldBox =lodNode.getWorldBox();
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

    void frameDraw()
    {
        const Pipe* pipe = static_cast< Pipe* >( _channel->getPipe( ));
        const uint32_t frame =
                pipe->getFrameData()->getFrameSettings().getFrameNumber();

        if( frame >= INVALID_TIMESTEP )
            return;

        applyCamera();
        const Frustum& frustum = setupFrustum();
        _frameInfo = FrameInfo( frustum, frame, pipe->getCurrentFrame( ));

        const eq::PixelViewport& pixVp = _channel->getPixelViewport();
        _drawRange = _channel->getRange();

        const eq::Viewport& vp = _channel->getViewport();
        _drawRange = _channel->getRange();

        const livre::Window* window = static_cast< const livre::Window* >( _channel->getWindow( ));
        const RenderPipeline& renderPipeline = window->getRenderPipeline();

        _renderer->update( *pipe->getFrameData( ));
        renderPipeline.render( { pipe->getFrameData()->getVRParameters(),
                                 _frameInfo,
                                 {{ _drawRange.start, _drawRange.end }},
                                 pipe->getFrameData()->getVolumeSettings().getDataSourceRange(),
                                 PixelViewport( pixVp.x, pixVp.y, pixVp.w, pixVp.h ),
                                 Viewport( vp.x, vp.y, vp.w, vp.h ),
                                 pipe->getFrameData()->getRenderSettings().getClipPlanes(),
                               },
                               PipeFilterT< RedrawFilter >( "RedrawFilter", _channel ),
                               PipeFilterT< SendHistogramFilter >( "SendHistogramFilter", _channel ),
                               *_renderer,
                               _availability );
    }

    void applyCamera()
    {
        const CameraSettings& cameraSettings = getFrameData()->getCameraSettings();
        glMultMatrixf( cameraSettings.getModelViewMatrix().array );
    }

    void configInit()
    {
        initializeFrame();
        initializeRenderer();
    }

    void configExit()
    {
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

#ifdef LIVRE_USE_ZEROEQ
        const size_t all = _availability.nAvailable + _availability.nNotAvailable;
        if( all > 0 )
        {
            _progress.restart( all );
            _progress += _availability.nAvailable;
            _publisher.publish( _progress );
        }
#endif
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
        const size_t all = _availability.nAvailable + _availability.nNotAvailable;
        const size_t missing = _availability.nNotAvailable;
        const float done = all > 0 ? float( all - missing ) / float( all ) : 0;
        const Window* window = static_cast< Window* >( _channel->getWindow( ));

        std::ostringstream os;
        os << node->getDataCache().getStatistics() << "  "
           << int( 100.f * done + .5f ) << "% loaded" << std::endl
           << window->getTextureCache().getStatistics();

        const DataSource& dataSource = node->getDataSource();
        const VolumeInformation& info = dataSource.getVolumeInfo();
        const Vector3f resolution = info.resolution;

        const float ratio = 1.0f / info.meterToDataUnitRatio;
        std::string unit = "unit unknown";
        if( ratio == 1e-6 )
            unit = "um";
        else if( ratio == 1e-3 )
            unit = "mm";
        else if( ratio == 1.0f )
            unit = "m";

        const size_t nBricks = _renderer->getNumBricksUsed();
        const float mbBricks =
            float( dataSource.getVolumeInfo().maximumBlockSize.product( )) /
            1024.f / 1024.f * float( nBricks );
        os << nBricks << " bricks / " << mbBricks << " MB rendered" << std::endl
           << "Total resolution " << info.voxels << " depth "
           << info.rootNode.getDepth() << std::endl
           << "Block resolution " << info.maximumBlockSize << std::endl;
        if( resolution.find_min() > 0.0f )
            os << resolution << "voxel/" << unit;

        float y = 240.f;
        std::string text = os.str();
        const eq::util::BitmapFont* font = _channel->getWindow()->getSmallFont();
        for( size_t pos = text.find( '\n' ); pos != std::string::npos;
             pos = text.find( '\n' ))
        {
            glRasterPos3f( 10.f, y, 0.99f );

            font->draw( text.substr( 0, pos ));
            text = text.substr( pos + 1 );
            y -= 16.f;
        }
        // last line might not end with /n
        glRasterPos3f( 10.f, y, 0.99f );
        font->draw( text );
    }

    void frameReadback( const eq::Frames& frames ) const
    {
        for( eq::Frame* frame: frames ) // Drop depth buffer from output frames
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
        const Matrix4f& modelviewIM = modelView.inverse();
        const Matrix3f& modelviewITM = vmml::transpose( Matrix3f( modelviewIM ));

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

    livre::Channel* _channel;
    eq::Range _drawRange;
    eq::Frame _frame;
    FrameGrabber _frameGrabber;
    FrameInfo _frameInfo;
    NodeAvailability _availability;
    std::unique_ptr< RayCastRenderer > _renderer;
    ::lexis::data::Progress _progress;
#ifdef LIVRE_USE_ZEROEQ
    zeroeq::Publisher _publisher;
#endif
};

void EqRaycastRenderer::_onFrameStart( const Frustum& frustum,
                                       const ClipPlanes& planes,
                                       const PixelViewport& view,
                                       const NodeIds& renderBricks )
{
    _channel.updateRegions( renderBricks, frustum );
    RayCastRenderer::_onFrameStart( frustum, planes, view, renderBricks );
}

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
    if( !eq::Channel::configInit( initId ))
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
                          const uint32_t frameCounter )
{
    eq::Channel::frameStart( frameID, frameCounter );
    _impl->_drawRange = eq::Range::ALL;
}

void Channel::frameDraw( const lunchbox::uint128_t& frameId )
{
    eq::Channel::frameDraw( frameId );
    _impl->frameDraw();
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
    std::stringstream filename;
    filename << std::setw( 5 ) << std::setfill('0')
             << _impl->_frameInfo.timeStep << ".png";
    return filename.str();
}


}
