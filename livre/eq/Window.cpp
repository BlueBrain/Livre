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

#include <livre/eq/Config.h>
#include <livre/eq/Error.h>
#include <livre/eq/Event.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/Node.h>
#include <livre/eq/Pipe.h>
#include <livre/eq/render/EqContext.h>
#include <livre/eq/settings/FrameSettings.h>

#include <livre/lib/cache/TextureCache.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>
#include <livre/lib/configuration/ClientParameters.h>

#include <livre/core/version.h>
#include <livre/core/pipeline/SimpleExecutor.h>
#include <livre/core/pipeline/Workers.h>

#include <eq/gl.h>

const uint32_t maxQueueSize = 65536;

namespace livre
{

class Window::Impl
{
public:
    explicit Impl( Window* window )
        : _window( window )
    {}

    void configInitGL()
    {
        _windowContext.reset( new EqContext( _window ));
        GLContext::glewSetContext( _window->glewGetContext( ));

        Pipe* pipe = static_cast< Pipe* >( _window->getPipe( ));

        const size_t computeThreadsPerWindow =
                pipe->getFrameData()->getClientParameters()->uploadThreadsPerWindow;

        const size_t uploadThreadsPerWindow =
                pipe->getFrameData()->getClientParameters()->uploadThreadsPerWindow;

        _computeExecutor.reset( new SimpleExecutor( WorkersPtr( new Workers( computeThreadsPerWindow,
                                                                             _windowContext ))));
        _uploadExecutor.reset( new SimpleExecutor( WorkersPtr( new Workers( uploadThreadsPerWindow,
                                                                             _windowContext ))));
    }

    void configInit()
    {
        Pipe* pipe = static_cast< Pipe* >( _window->getPipe( ));
        const size_t maxGpuMemory =
                pipe->getFrameData()->getVRParameters()->maxGPUCacheMemoryMB;
       _textureCache.reset( new TextureCache( maxGpuMemory * LB_1MB, GL_LUMINANCE8 ));
    }

    Window* const _window;
    GLContextPtr _windowContext;
    CachePtr _textureCache;
    ExecutorPtr _computeExecutor;
    ExecutorPtr _uploadExecutor;
};

Window::Window( eq::Pipe *parent )
    : eq::Window( parent )
    , _impl( new Window::Impl( this ))
{
}

ExecutorPtr Window::getComputeExecutor()
{
    return _impl->_computeExecutor;
}

ExecutorPtr Window::getUploadExecutor()
{
    return _impl->_uploadExecutor;
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
    return eq::Window::configExitGL();
}

void Window::frameStart( const eq::uint128_t& frameID,
                         const uint32_t frameNumber )
{
    eq::Window::frameStart( frameID, frameNumber );
}

CachePtr Window::getTextureCache()
{
    return _impl->_textureCache;
}

ConstCachePtr Window::getTextureCache() const
{
    return _impl->_textureCache;
}

}
