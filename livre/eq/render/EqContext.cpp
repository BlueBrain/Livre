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

#include <livre/eq/Pipe.h>
#include <livre/eq/Window.h>
#include <livre/eq/render/EqContext.h>

namespace livre
{
namespace
{
}

EqContext::EqContext(Window* const window)
    : GLContext(window->glewGetContext())
    , _window(window)
    , _systemWindow(window->getSystemWindow())
{
    if (!_systemWindow)
        throw std::runtime_error("Missing system window");
}

EqContext::EqContext(const EqContext& shared)
    : GLContext(shared._window->glewGetContext())
    , _window(shared._window)
    , _systemWindow(nullptr)
{
    eq::WindowSettings settings = _window->getSettings();
    settings.setSharedContextWindow(_window->getSystemWindow());
    settings.setIAttribute(eq::WindowSettings::IATTR_HINT_DRAWABLE, eq::OFF);
    const eq::Pipe* pipe = _window->getPipe();

    _window->getSystemWindow()->doneCurrent();
    _newSystemWindow.reset(
        pipe->getWindowSystem().createWindow(_window, settings));
    if (_newSystemWindow->configInit())
        _systemWindow = _newSystemWindow.get();
    else
        _newSystemWindow.reset();
    if (!_systemWindow)
        throw std::runtime_error("Missing system window in clone");
}

EqContext::~EqContext()
{
}

GLContextPtr EqContext::clone() const
{
    return GLContextPtr(new EqContext(*this));
}

void EqContext::makeCurrent()
{
    if (_systemWindow)
    {
        GLContext::makeCurrent();
        _systemWindow->makeCurrent();
    }
}

void EqContext::doneCurrent()
{
    if (_systemWindow)
    {
        GLContext::doneCurrent();
        _systemWindow->doneCurrent();
    }
}
}
