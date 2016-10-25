/* Copyright (c) 2007-2016, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <livre/eq/settings/EqFrameSettings.h>

#include <livre/core/configuration/RendererParameters.h>
#include <livre/core/render/RenderPipeline.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/render/TexturePool.h>

#include <eq/gl.h>

namespace livre
{

struct Window::Impl
{
public:
    explicit Impl( Window* window )
        : _window( window )
    {}

    void configInitGL()
    {
        _glContext.reset( new EqContext( _window ));
    }

    bool configExitGL()
    {
        _glContext->doneCurrent();
        _glContext.reset();
        _textureCache->purge();
        return true;
    }

    void configInit()
    {
        shareGLContexts();
        _renderPipeline.reset( new RenderPipeline( "gl" ));
    }

    void shareGLContexts()
    {
        const Window* sharedWindow = static_cast< const Window* >(
                                         _window->getSharedContextWindow( ));

        if( sharedWindow && sharedWindow != _window )
        {
            _glContext = sharedWindow->_impl->_glContext;
            return;
        }
    }

    Window* const _window;
    GLContextPtr _glContext;
    std::unique_ptr< TexturePool > _texturePool;
    std::unique_ptr< Cache > _textureCache;
    std::unique_ptr< RenderPipeline > _renderPipeline;
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
    return _impl->configExitGL();
}

Cache& Window::getTextureCache()
{
    return *_impl->_textureCache;
}

const Cache& Window::getTextureCache() const
{
    return *_impl->_textureCache;
}

RenderPipeline& Window::getRenderPipeline() const
{
    return *_impl->_renderPipeline;
}

}
