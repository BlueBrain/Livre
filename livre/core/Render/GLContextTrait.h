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

#ifndef _GLContextTrait_h_
#define _GLContextTrait_h_

#include <livre/core/types.h>

namespace livre
{

/**
 * The GLProcessor class is based on the \see Processor class. It includes a context based on OpenGL
 */
class GLContextTrait
{
public:

   virtual ~GLContextTrait() { }

    /**
     * Sets the GL context.
     * @param glContextPtr context ptr.
     */
    void setGLContext( GLContextPtr glContextPtr );

    /**
     * @return The GL context.
     */
    GLContextPtr getGLContext( );

    /**
     * @return The GL context.
     */
    ConstGLContextPtr getGLContext( ) const;

protected:

    /**
     * Is called before the dash context is set.
     * @return Context is not set afterwards if false is returned.
     */
    virtual bool onPreSetGLContext_( );

    /**
     * Is called when the dash context is set.
     */
    virtual void onSetGLContext_( ) {}

    GLContextPtr glContextPtr_;

};


}

#endif // _Processor_h_
