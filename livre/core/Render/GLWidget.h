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

#ifndef _GLWidget_h_
#define _GLWidget_h_

#include <livre/core/mathTypes.h>
#include <livre/core/Render/GLContextTrait.h>

namespace livre
{

/**
 * RenderWidget class is the base class for renderWidgets, that has the information of the
 * window of rendering.
 */
class GLWidget : public GLContextTrait
{
public:

    void renderView( View* view,
                     RenderingSetGenerator& renderSetGenerator );

    virtual void setViewport( const View* view, Viewporti& viewport ) const;

    virtual uint32_t getX( ) const = 0;

    virtual uint32_t getY( ) const = 0;

    virtual uint32_t getHeight( ) const = 0;

    virtual uint32_t getWidth( ) const = 0;

    virtual void update( ) { }

protected:

    GLWidget( );

    virtual ~GLWidget( );
};

}
#endif // _GLWidget_h_
