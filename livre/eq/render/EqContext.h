/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
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

#ifndef _EqContext_h_
#define _EqContext_h_

#include <livre/core/render/GLContext.h>

namespace livre
{
/**
 * The EqContext class for wrapping the GLContext for equalizer library.
 */
class EqContext : public GLContext
{
public:
    /**
     * @param window is the parent equalizer window that has the rendering
     * context.
     */
    EqContext(Window* const window);

    ~EqContext();

    /**
     * @copydoc GLContext::clone
     */
    GLContextPtr clone() const final;

    /**
     * @copydoc GLContext::makeCurrent
     */
    virtual void makeCurrent() final;

    /**
     * @copydoc GLContext::doneCurrent
     */
    virtual void doneCurrent() final;

private:
    EqContext() = delete;
    EqContext(const EqContext& shared);

    Window* const _window;
    eq::SystemWindow* _systemWindow;
    std::unique_ptr<eq::SystemWindow> _newSystemWindow;
};
}

#endif // _EqContext_h_
