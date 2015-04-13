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

#ifndef _GLContext_h_
#define _GLContext_h_

#include <livre/core/types.h>
#include <livre/core/lunchboxTypes.h>

namespace livre
{

/**
 * The GLContext class is the warpper for different kinds of OpenGL contexts
 */
class GLContext : public boost::enable_shared_from_this< GLContext >
{
public:

    GLContext( );

    virtual ~GLContext( );

    /**
     * Shares the current context with the given context.
     * @param dstContextPtr The destination context to share the context with. It is also added to the list.
     */
    void shareContext( GLContextPtr dstContextPtr );

    /**
     * Makes the context current.
     */
    virtual void makeCurrent( ) = 0;

    /**
     * Clears the current context.
     */
    virtual void doneCurrent( ) = 0;

    /**
     * Gets the current context
     */
    static const GLContext* getCurrent();

protected:

   /**
     * Implements the sharing of the context from the srcSharedContext.
     * @param srcSharedContext The source context.
     */
    virtual void shareContext_( GLContext* srcSharedContext ) = 0;

private:

    GLContextPtr parent_;
};

}

#endif // _GLContext_h_
