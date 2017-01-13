/* Copyright (c) 2007-2017, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <livre/lib/configuration/VolumeRendererParameters.h>
#include <livre/lib/pipeline/RenderPipeline.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/data/DataSource.h>
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

        shareGLContexts();

        Node* node = static_cast< Node* >( _window->getNode( ));
        Pipe* pipe = static_cast< Pipe* >( _window->getPipe( ));
        const size_t maxGpuMemory =
                        pipe->getFrameData().getVRParameters().getMaxGPUCacheMemoryMB();

        _texturePool.reset( new TexturePool( node->getDataSource( )));
        _textureCache.reset( new CacheT< TextureObject >( "TextureCache", maxGpuMemory * LB_1MB ));
        Caches caches = { node->getDataCache(), *_textureCache, node->getHistogramCache() };
        _renderPipeline.reset( new RenderPipeline( node->getDataSource(),
                                                   caches,
                                                   *_texturePool,
                                                   _glContext ));
    }

    bool configExitGL()
    {
        _renderPipeline.reset();
        _textureCache.reset();
        _texturePool.reset();

        if( _glContext.use_count() == 1 )
        {
            Node* node = static_cast< Node* >( _window->getNode( ));
            node->getDataSource().finishGL();
        }

        _glContext->doneCurrent();
        _glContext.reset();
        return true;
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
    setIAttribute( eq::WindowSettings::IATTR_PLANES_DEPTH, 0 );
    return eq::Window::configInit( initId );
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

const RenderPipeline& Window::getRenderPipeline() const
{
    return *_impl->_renderPipeline;
}

}
