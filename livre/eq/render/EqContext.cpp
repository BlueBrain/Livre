/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#include <eq/eq.h>

#include <livre/eq/render/EqContext.h>
#include <livre/eq/Pipe.h>
#include <livre/eq/Window.h>

namespace livre
{

namespace
{
boost::mutex glContextMutex;
}

EqContext::EqContext( Window* const window )
    : GLContext()
    , _window( window )
    , _systemWindow( 0 )
{}

EqContext::~EqContext()
{
    delete _systemWindow;
}

void EqContext::share( const GLContext& src )
{
    LBASSERT( _window );
    ScopedLock lock( glContextMutex );

    // Context is already created so return.
    if( _systemWindow )
        return;

    const EqContext* parent = dynamic_cast< const EqContext* >( &src );
    if( !parent )
    {
        LBERROR << "Only same kind of contexts can be shared" << std::endl;
        return;
    }

    if( !parent->_window )
    {
        LBERROR << "Parent context can not be NULL" << std::endl;
        return;
    }

    eq::WindowSettings settings = _window->getSettings();
    settings.setSharedContextWindow( _window->getSystemWindow() );
    settings.setIAttribute( eq::WindowSettings::IATTR_HINT_DRAWABLE, eq::OFF );
    const eq::Pipe* pipe = _window->getPipe();
    _systemWindow = pipe->getWindowSystem().createWindow( _window, settings );

    if( !_systemWindow->configInit( ))
    {
        delete _systemWindow;
        _systemWindow = 0;
    }
}

GLContextPtr EqContext::clone() const
{
    return GLContextPtr( new EqContext( _window ));
}

void EqContext::makeCurrent()
{
    if( _systemWindow )
    {
        GLContext::makeCurrent();
        _systemWindow->makeCurrent();
    }
}

void EqContext::doneCurrent()
{
    if( _systemWindow )
    {
        GLContext::doneCurrent();
        _systemWindow->doneCurrent();
    }
}

}
