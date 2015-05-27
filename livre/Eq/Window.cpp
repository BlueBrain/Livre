/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Maxim Makhinya  <maxmah@gmail.com>
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
#include <eq/gl.h>

#include <livre/Eq/Window.h>
#include <livre/Eq/Error.h>


namespace livre
{

Window::Window( eq::Pipe *parent )
    : eq::Window( parent )
{
    LBASSERT( parent );
}

bool Window::configInit( const eq::uint128_t& initId )
{
    // Enforce alpha channel, since we need one for rendering
    setIAttribute( eq::WindowSettings::IATTR_PLANES_ALPHA, 8 );

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

    return eq::Window::configInitGL( initId );
}


}
