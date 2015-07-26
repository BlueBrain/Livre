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

#include <livre/Eq/Window.h>

#include <livre/core/version.h>
#include <livre/Eq/Error.h>
#include <livre/Eq/FrameData.h>
#include <livre/Eq/Node.h>
#include <livre/Eq/Pipe.h>
#include <livre/Eq/Render/EqContext.h>
#include <livre/Eq/Settings/FrameSettings.h>

#include <livre/core/Dash/DashTree.h>
#include <livre/core/DashPipeline/DashConnection.h>
#include <livre/core/DashPipeline/DashProcessor.h>
#include <livre/core/DashPipeline/DashProcessorInput.h>
#include <livre/core/DashPipeline/DashProcessorOutput.h>

#include <livre/Lib/Configuration/VolumeRendererParameters.h>

#include <livre/Lib/Uploaders/DataUploadProcessor.h>
#include <livre/Lib/Uploaders/TextureUploadProcessor.h>

#include <eq/gl.h>

const uint32_t maxQueueSize = 65536;

namespace livre
{

class Window::Impl
{
public:
    Impl( Window* window )
        : _window( window )
        , _dashProcessorPtr( new DashProcessor( ))
    {}

    void configInitGL()
    {
        const Window* sharedWindow = static_cast< const Window* >( _window->getSharedContextWindow( ));

        // share upload processors for windows which also share the GL context
        if( sharedWindow && sharedWindow != _window )
        {
            _windowContext = sharedWindow->_impl->_windowContext;
            _dashProcessorPtr = sharedWindow->_impl->_dashProcessorPtr;
            _textureUploadProcessorPtr = sharedWindow->_impl->_textureUploadProcessorPtr;
            _dataUploadProcessorPtr = sharedWindow->_impl->_dataUploadProcessorPtr;
            return;
        }
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
        LBASSERT( _dataUploadProcessorPtr.use_count() == _textureUploadProcessorPtr.use_count( ));
        if( !_dataUploadProcessorPtr.unique( ))
            return;

        if( !_dashProcessorPtr->getDashContext( ))
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
        _dashProcessorPtr->getDashContext()->setCurrent();
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
        if( !_textureUploadProcessorPtr->isRunning( ))
            _textureUploadProcessorPtr->start();

        if( !_dataUploadProcessorPtr->isRunning( ))
            _dataUploadProcessorPtr->start();
#ifdef _MSC_VER
        _window->makeCurrent( false );
#endif
    }

    void stopUploadProcessors()
    {
        commit();
        _textureUploadProcessorPtr->join();
        _dataUploadProcessorPtr->join();
    }

    void commit()
    {
        _dashProcessorPtr->getProcessorOutput_()->commit( CONNECTION_ID );
    }

    void apply()
    {
        Pipe* pipe = static_cast< Pipe* >( _window->getPipe( ));
        ProcessorInputPtr input = _dashProcessorPtr->getProcessorInput_();

        // #75: only wait for data in synchronous mode
        if( pipe->getFrameData()->getVRParameters()->synchronousMode ||
            input->dataWaitingOnInput( CONNECTION_ID ))
        {
            input->applyAll( CONNECTION_ID );
        }
    }

    void initializePipelineProcessors()
    {
        Node* node = static_cast< Node* >( _window->getNode( ));
        DashTreePtr dashTree = node->getDashTree();
        _dashProcessorPtr->setDashContext( dashTree->createContext( ));

        GLContextPtr dataUploadContext( new EqContext( _window ));
        _dataUploadProcessorPtr.reset(
            new DataUploadProcessor( dashTree, _windowContext,
                                     dataUploadContext,
                                     node->getTextureDataCache( )));

        GLContextPtr textureUploadContext( new EqContext( _window ));
        Pipe* pipe = static_cast< Pipe* >( _window->getPipe( ));
        _textureUploadProcessorPtr.reset(
            new TextureUploadProcessor( dashTree, _windowContext,
                                        textureUploadContext,
                                        pipe->getFrameData()->getVRParameters( )));
    }

    void releasePipelineProcessors()
    {
        stopUploadProcessors();
        _textureUploadProcessorPtr->setDashContext( DashContextPtr( ));
        _dataUploadProcessorPtr->setDashContext( DashContextPtr( ));
        _textureUploadProcessorPtr.reset();
        _dataUploadProcessorPtr.reset();
        _dashProcessorPtr->setDashContext( DashContextPtr( ));
    }

    void initializePipelineConnections()
    {
        // Connects data uploader to texture uploader
        DashConnectionPtr dataOutputConnectionPtr( new DashConnection( maxQueueSize ) );
        _dataUploadProcessorPtr->getProcessorOutput_< DashProcessorOutput >( )
                ->addConnection( CONNECTION_ID, dataOutputConnectionPtr );

        _textureUploadProcessorPtr->getProcessorInput_< DashProcessorInput >( )
                ->addConnection( CONNECTION_ID, dataOutputConnectionPtr );

        // Connects texture uploader to pipe
        DashConnectionPtr texOutputConnectionPtr( new DashConnection( maxQueueSize ) );
        _textureUploadProcessorPtr->getProcessorOutput_< DashProcessorOutput >()
                ->addConnection( CONNECTION_ID, texOutputConnectionPtr );
        _dashProcessorPtr->getProcessorInput_< DashProcessorInput >()
                ->addConnection( CONNECTION_ID, texOutputConnectionPtr );
        // Connects pipe to data uploader
        DashConnectionPtr pipeOutputConnectionPtr( new DashConnection( maxQueueSize ) );
        _dataUploadProcessorPtr->getProcessorInput_< DashProcessorInput >()
                ->addConnection( CONNECTION_ID, pipeOutputConnectionPtr );
        _dashProcessorPtr->getProcessorOutput_< DashProcessorOutput >()
                ->addConnection( CONNECTION_ID, pipeOutputConnectionPtr );
    }

    void releasePipelineConnections( )
    {
        _dashProcessorPtr->getProcessorOutput_< DashProcessorOutput >()
                ->removeConnection( CONNECTION_ID );
        _dataUploadProcessorPtr->getProcessorInput_< DashProcessorInput >()
                ->removeConnection( CONNECTION_ID );
        _dashProcessorPtr->getProcessorInput_< DashProcessorInput >( )
                ->removeConnection( CONNECTION_ID );
        _textureUploadProcessorPtr->getProcessorOutput_< DashProcessorOutput >( )
                ->removeConnection( CONNECTION_ID );
        _textureUploadProcessorPtr->getProcessorInput_< DashProcessorInput >( )
                ->removeConnection( CONNECTION_ID );
        _dataUploadProcessorPtr->getProcessorOutput_< DashProcessorOutput >( )
                ->removeConnection( CONNECTION_ID );
    }

    Window* const _window;
    TextureUploadProcessorPtr _textureUploadProcessorPtr;
    DataUploadProcessorPtr _dataUploadProcessorPtr;
    DashProcessorPtr _dashProcessorPtr;
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
    if( eq::Window::configInit( initId ))
    {
        _impl->configInit();
        return true;
    }

    return false;
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

void Window::apply()
{
    _impl->apply();
}

const TextureCache& Window::getTextureCache() const
{
    return _impl->_textureUploadProcessorPtr->getTextureCache();
}

}
