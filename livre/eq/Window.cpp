/* Copyright (c) 2007-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
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

#include <livre/eq/Window.h>

#include <livre/core/version.h>
#include <livre/eq/Config.h>
#include <livre/eq/Error.h>
#include <livre/eq/Event.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/Node.h>
#include <livre/eq/Pipe.h>
#include <livre/eq/render/EqContext.h>
#include <livre/eq/settings/FrameSettings.h>

#include <livre/core/dash/DashTree.h>
#include <livre/core/dashpipeline/DashConnection.h>
#include <livre/core/dashpipeline/DashProcessor.h>
#include <livre/core/dashpipeline/DashProcessorInput.h>
#include <livre/core/dashpipeline/DashProcessorOutput.h>

#include <livre/lib/configuration/VolumeRendererParameters.h>

#include <livre/lib/uploaders/DataUploadProcessor.h>
#include <livre/lib/uploaders/TextureUploadProcessor.h>

#include <eq/gl.h>

const uint32_t maxQueueSize = 65536;

namespace livre
{

class EqTextureUploadProcessor : public TextureUploadProcessor
{
public:
    EqTextureUploadProcessor( Config& config,
                              DashTreePtr dashTree,
                              GLContextPtr shareContext,
                              GLContextPtr context,
                              ConstVolumeRendererParametersPtr parameters )
        : TextureUploadProcessor( dashTree, shareContext, context, parameters )
        , _config( config )
    {}

    void onPostCommit_( uint32_t connection, CommitState state ) final
    {
        TextureUploadProcessor::onPostCommit_( connection, state );
        if( needRedraw( ))
            _config.sendEvent( REDRAW );
    }

private:
    Config& _config;
};

class Window::Impl
{
public:
    explicit Impl( Window* window )
        : _window( window )
        , _dashProcessor( new DashProcessor( ))
    {}

    void configInitGL()
    {
        _windowContext.reset( new EqContext( _window ));
        GLContext::glewSetContext( _window->glewGetContext( ));
    }

    void configInit()
    {
        initializePipelineProcessors();
        initializePipelineConnections();
        startUploadProcessors();
    }

    void configExitGL()
    {
        // if we are the last window of a share group, release the processors
        LBASSERT( _dataUploader.use_count() == _textureUploader.use_count( ));
        if( !_dataUploader.unique() || !_dashProcessor->getDashContext( ))
            return;

        releasePipelineConnections();
        releasePipelineProcessors();
    }

    void configExit()
    {
        livre::Node* node = static_cast< livre::Node* >( _window->getNode( ));
        node->getDashTree()->getRenderStatus().setThreadOp( TO_EXIT );
    }

    void frameStart()
    {
        _dashProcessor->getDashContext()->setCurrent();
        livre::Node* node = static_cast< livre::Node* >( _window->getNode( ));
        DashRenderStatus& renderStatus = node->getDashTree()->getRenderStatus();

        Pipe* pipe = static_cast< Pipe* >( _window->getPipe( ));

        renderStatus.setFrameID(
            pipe->getFrameData()->getFrameSettings()->getFrameNumber( ));
    }

    void startUploadProcessors()
    {
#ifdef _MSC_VER
        // https://www.opengl.org/discussion_boards/showthread.php/152648-wglShareLists-failing
        LBCHECK( wglMakeCurrent( 0,0 ));
#endif
        if( !_textureUploader->isRunning( ))
            _textureUploader->start();

        if( !_dataUploader->isRunning( ))
            _dataUploader->start();
#ifdef _MSC_VER
        _window->makeCurrent( false );
#endif
    }

    void stopUploadProcessors()
    {
        commit();
        _textureUploader->join();
        _dataUploader->join();
    }

    void commit()
    {
        _dashProcessor->getProcessorOutput_()->commit( CONNECTION_ID );
    }

    bool apply( bool wait )
    {
        ProcessorInputPtr input = _dashProcessor->getProcessorInput_();

        if( wait || input->dataWaitingOnInput( CONNECTION_ID ))
            return input->applyAll( CONNECTION_ID );

        return false;
    }

    void initializePipelineProcessors()
    {
        const Window* sharedWindow = static_cast< const Window* >(
                                         _window->getSharedContextWindow( ));

        // share upload processors for windows which also share the GL context
        if( sharedWindow && sharedWindow != _window )
        {
            _windowContext = sharedWindow->_impl->_windowContext;
            _dashProcessor = sharedWindow->_impl->_dashProcessor;
            _textureUploader = sharedWindow->_impl->_textureUploader;
            _dataUploader = sharedWindow->_impl->_dataUploader;
            return;
        }

        // First one in group: setup
        Node* node = static_cast< Node* >( _window->getNode( ));
        DashTreePtr dashTree = node->getDashTree();
        _dashProcessor->setDashContext( dashTree->createContext( ));

        GLContextPtr dataUploadContext( new EqContext( _window ));
        _dataUploader.reset( new DataUploadProcessor( dashTree, _windowContext,
                                                      dataUploadContext,
                                                 node->getTextureDataCache( )));

        GLContextPtr textureUploadContext( new EqContext( _window ));
        Config* config = static_cast< Config* >( _window->getConfig( ));
        Pipe* pipe = static_cast< Pipe* >( _window->getPipe( ));
        _textureUploader.reset(
            new EqTextureUploadProcessor( *config, dashTree, _windowContext,
                                          textureUploadContext,
                                     pipe->getFrameData()->getVRParameters( )));
    }

    void releasePipelineProcessors()
    {
        stopUploadProcessors();
        _textureUploader->setDashContext( DashContextPtr( ));
        _dataUploader->setDashContext( DashContextPtr( ));
        _textureUploader.reset();
        _dataUploader.reset();
        _dashProcessor->setDashContext( DashContextPtr( ));
    }

    void initializePipelineConnections()
    {
        // Connects data uploader to texture uploader
        DashConnectionPtr dataOutputConnectionPtr( new DashConnection( maxQueueSize ) );
        _dataUploader->getProcessorOutput_< DashProcessorOutput >( )
                ->addConnection( CONNECTION_ID, dataOutputConnectionPtr );

        _textureUploader->getProcessorInput_< DashProcessorInput >( )
                ->addConnection( CONNECTION_ID, dataOutputConnectionPtr );

        // Connects texture uploader to pipe
        DashConnectionPtr texOutputConnectionPtr( new DashConnection( maxQueueSize ) );
        _textureUploader->getProcessorOutput_< DashProcessorOutput >()
                ->addConnection( CONNECTION_ID, texOutputConnectionPtr );
        _dashProcessor->getProcessorInput_< DashProcessorInput >()
                ->addConnection( CONNECTION_ID, texOutputConnectionPtr );
        // Connects pipe to data uploader
        DashConnectionPtr pipeOutputConnectionPtr( new DashConnection( maxQueueSize ) );
        _dataUploader->getProcessorInput_< DashProcessorInput >()
                ->addConnection( CONNECTION_ID, pipeOutputConnectionPtr );
        _dashProcessor->getProcessorOutput_< DashProcessorOutput >()
                ->addConnection( CONNECTION_ID, pipeOutputConnectionPtr );
    }

    void releasePipelineConnections( )
    {
        _dashProcessor->getProcessorOutput_< DashProcessorOutput >()
                ->removeConnection( CONNECTION_ID );
        _dataUploader->getProcessorInput_< DashProcessorInput >()
                ->removeConnection( CONNECTION_ID );
        _dashProcessor->getProcessorInput_< DashProcessorInput >( )
                ->removeConnection( CONNECTION_ID );
        _textureUploader->getProcessorOutput_< DashProcessorOutput >( )
                ->removeConnection( CONNECTION_ID );
        _textureUploader->getProcessorInput_< DashProcessorInput >( )
                ->removeConnection( CONNECTION_ID );
        _dataUploader->getProcessorOutput_< DashProcessorOutput >( )
                ->removeConnection( CONNECTION_ID );
    }

    Window* const _window;
    TextureUploadProcessorPtr _textureUploader;
    DataUploadProcessorPtr _dataUploader;
    DashProcessorPtr _dashProcessor;
    GLContextPtr _windowContext;
};

Window::Window( eq::Pipe *parent )
    : eq::Window( parent )
    , _impl( new Window::Impl( this ))
{
}

bool Window::configInit( const eq::uint128_t& initId )
{
    std::stringstream windowTitle;
    windowTitle << "Livre " << livrecore::Version::getString();
    setName( windowTitle.str( ));

    // Enforce alpha channel, since we need one for rendering
    setIAttribute( eq::WindowSettings::IATTR_PLANES_ALPHA, 8 );
    if( !eq::Window::configInit( initId ))
        return false;

    _impl->configInit();
    return true;
}

bool Window::configExit()
{
    _impl->configExit();
    return eq::Window::configExit();
}

bool Window::configInitGL( const eq::uint128_t& initId )
{
    if( !GLEW_ARB_shader_objects )
    {
        sendError( ERROR_LIVRE_ARB_SHADER_OBJECTS_MISSING );
        return false;
    }
    if( !GLEW_EXT_blend_func_separate )
    {
        sendError( ERROR_LIVRE_EXT_BLEND_FUNC_SEPARATE_MISSING );
        return false;
    }
    if( !GLEW_ARB_multitexture )
    {
        sendError( ERROR_LIVRE_ARB_MULTITEXTURE_MISSING );
        return false;
    }

    glDisable( GL_DEPTH_TEST );

    if( !eq::Window::configInitGL( initId ))
        return false;
    _impl->configInitGL();
    return true;
}

bool Window::configExitGL()
{
    _impl->configExitGL();
    return eq::Window::configExitGL();
}

void Window::frameStart( const eq::uint128_t& frameID,
                         const uint32_t frameNumber )
{
    _impl->frameStart();
    eq::Window::frameStart( frameID, frameNumber );
}

void Window::commit()
{
    _impl->commit();
}

bool Window::apply( bool wait )
{
    return _impl->apply( wait );
}

const TextureCache& Window::getTextureCache() const
{
    return _impl->_textureUploader->getTextureCache();
}

}
