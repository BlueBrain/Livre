/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#ifndef _GLWidget_h_
#define _GLWidget_h_

#include <livre/core/api.h>
#include <livre/core/mathTypes.h>
#include <livre/core/render/GLContextTrait.h>

namespace livre
{

/** Base class for render widgets */
class GLWidget : public GLContextTrait
{
public:
    virtual Viewport getViewport( const View& view ) const;

    virtual uint32_t getX( ) const = 0;
    virtual uint32_t getY( ) const = 0;
    virtual uint32_t getHeight( ) const = 0;
    virtual uint32_t getWidth( ) const = 0;

    virtual void update( ) { }

protected:
    LIVRECORE_API GLWidget( );

    LIVRECORE_API virtual ~GLWidget( );
};

}
#endif // _GLWidget_h_
