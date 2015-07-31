/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/Eq/Render/EqContext.h>
#include <livre/Eq/Pipe.h>
#include <livre/Eq/Window.h>

namespace livre
{

EqContext::EqContext( Window* const window )
    : GLContext()
    , window_( window )
    , systemWindow_( 0 )
{
}

EqContext::~EqContext()
{
    delete systemWindow_;
}

void EqContext::makeCurrent()
{
    if( systemWindow_ )
        systemWindow_->makeCurrent();
}

void EqContext::doneCurrent()
{

}

void EqContext::shareContext_( GLContext* srcSharedContext )
{
    LBASSERT( window_ );

    // Context is already created so return.
    if( systemWindow_ )
        return;

    EqContext* parent = dynamic_cast< EqContext * >( srcSharedContext );
    if( !parent )
    {
        LBERROR << "Only same kind of contexts can be shared" << std::endl;
        return;
    }

    if( !parent->window_ )
    {
        LBERROR << "Parent context can not be NULL" << std::endl;
        return;
    }

    eq::WindowSettings settings = window_->getSettings();
    settings.setSharedContextWindow( window_->getSystemWindow() );
    settings.setIAttribute( eq::WindowSettings::IATTR_HINT_DRAWABLE, eq::OFF );
    const eq::Pipe* pipe = window_->getPipe();
    systemWindow_ = pipe->getWindowSystem().createWindow( window_, settings );

    if( !systemWindow_->configInit( ) )
    {
        delete systemWindow_;
        systemWindow_ = 0;
    }
}

}
