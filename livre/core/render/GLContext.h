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

#ifndef _GLContext_h_
#define _GLContext_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/lunchboxTypes.h>

namespace livre
{

/**
 * The GLContext class is the warpper for different kinds of OpenGL contexts
 */
class GLContext
{
public:
    LIVRECORE_API GLContext( const GLEWContext* glewContext );
    LIVRECORE_API virtual ~GLContext();

    /**
     * Shares the context with the source context.
     * @param src is the source context to share the context with
     */
    virtual void share( const GLContext& src ) = 0;

    /**
     * @return a clone of the context.
     */
    virtual GLContextPtr clone() const = 0;

    /**
     * Makes the context current.
     */
    virtual void makeCurrent();

    /**
     * Clears the current context.
     */
    virtual void doneCurrent() = 0;

    /**
     * Gets the current context
     */
    LIVRECORE_API static const GLContext* getCurrent();

    /** @return the global glew context, @see glewSetContext */
    LIVRECORE_API const GLEWContext* glewGetContext() const;

private:
    const GLEWContext* _glewContext;
};

}

#endif // _GLContext_h_
