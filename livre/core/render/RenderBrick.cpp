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

/* gluProject code used in getScreenCoordinates():
 *
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization from
 * Silicon Graphics, Inc.
 */
#include "RenderBrick.h"

#include <livre/core/render/Frustum.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/maths/maths.h>
#include <eq/gl.h>

namespace livre
{


RenderBrick::RenderBrick( const LODNode& lodNode,
                          const ConstTextureStatePtr& textureState )
    : _lodNode( lodNode )
    , _textureState( textureState )

{
}

void RenderBrick::drawBrick( bool front, bool back  ) const
{
    if( !front && !back )
        return;
    else if( front && !back )
    {
        glCullFace( GL_BACK );
    }
    else if( !front && back )
    {
        glCullFace( GL_FRONT );
    }

    const Boxf& worldBox = _lodNode.getWorldBox();
    const Vector3f& minPos = worldBox.getMin();
    const Vector3f& maxPos = worldBox.getMax();

    glBegin( GL_QUADS );
    {
        const float norm = -1.0f;
        glNormal3f(  norm, 0.0f, 0.0f );
        glVertex3f( minPos[ 0 ], minPos[ 1 ], minPos[ 2 ] ); // 0
        glVertex3f( minPos[ 0 ], minPos[ 1 ], maxPos[ 2 ] ); // 1
        glVertex3f( minPos[ 0 ], maxPos[ 1 ], maxPos[ 2 ] ); // 3
        glVertex3f( minPos[ 0 ], maxPos[ 1 ], minPos[ 2 ] ); // 2

        glNormal3f( 0.0f,  -norm, 0.0f );
        glVertex3f( minPos[ 0 ], maxPos[ 1 ], minPos[ 2 ] ); // 2
        glVertex3f( minPos[ 0 ], maxPos[ 1 ], maxPos[ 2 ] ); // 3
        glVertex3f( maxPos[ 0 ], maxPos[ 1 ], maxPos[ 2 ] ); // 5
        glVertex3f( maxPos[ 0 ], maxPos[ 1 ], minPos[ 2 ] ); // 4

        glNormal3f( -norm, 0.0f, 0.0f );
        glVertex3f( maxPos[ 0 ], maxPos[ 1 ], minPos[ 2 ] ); // 4
        glVertex3f( maxPos[ 0 ], maxPos[ 1 ], maxPos[ 2 ] ); // 5
        glVertex3f( maxPos[ 0 ], minPos[ 1 ], maxPos[ 2 ] ); // 7
        glVertex3f( maxPos[ 0 ], minPos[ 1 ], minPos[ 2 ] ); // 6

        glNormal3f( 0.0f,  norm, 0.0f );
        glVertex3f( maxPos[ 0 ], minPos[ 1 ], minPos[ 2 ] ); // 6
        glVertex3f( maxPos[ 0 ], minPos[ 1 ], maxPos[ 2 ] ); // 7
        glVertex3f( minPos[ 0 ], minPos[ 1 ], maxPos[ 2 ] ); // 1
        glVertex3f( minPos[ 0 ], minPos[ 1 ], minPos[ 2 ] ); // 0

        glNormal3f( 0.0f, 0.0f, -norm );
        glVertex3f( minPos[ 0 ], minPos[ 1 ], maxPos[ 2 ] ); // 1
        glVertex3f( maxPos[ 0 ], minPos[ 1 ], maxPos[ 2 ] ); // 7
        glVertex3f( maxPos[ 0 ], maxPos[ 1 ], maxPos[ 2 ] ); // 5
        glVertex3f( minPos[ 0 ], maxPos[ 1 ], maxPos[ 2 ] ); // 3

        glNormal3f( 0.0f, 0.0f,  norm );
        glVertex3f( minPos[ 0 ], minPos[ 1 ], minPos[ 2 ] ); // 0
        glVertex3f( minPos[ 0 ], maxPos[ 1 ], minPos[ 2 ] ); // 2
        glVertex3f( maxPos[ 0 ], maxPos[ 1 ], minPos[ 2 ] ); // 4
        glVertex3f( maxPos[ 0 ], minPos[ 1 ], minPos[ 2 ] ); // 6
   }
   glEnd();
}


}
