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


RenderBrick::RenderBrick( const ConstLODNodePtr& lodNode,
                          const ConstTextureStatePtr& textureState )
    : _lodNode( lodNode )
    , _textureState( textureState )

{
}

void RenderBrick::getScreenCoordinates( const Frustum& frustum,
                                        const PixelViewport& pvp,
                                        Vector2i& minScreenPos,
                                        Vector2i& maxScreenPos ) const
{
    const Vector3f& minPos = _lodNode->getWorldBox().getMin();
    const Vector3f& maxPos = _lodNode->getWorldBox().getMax();

    const double x[ 2 ] = { minPos[ 0 ], maxPos[ 0 ] };
    const double y[ 2 ] = { minPos[ 1 ], maxPos[ 1 ] };
    const double z[ 2 ] = { minPos[ 2 ], maxPos[ 2 ] };

    double xMax = -std::numeric_limits< double >::max();
    double yMax = -std::numeric_limits< double >::max();

    double xMin = -xMax;
    double yMin = -yMax;

    for( int32_t i = 0; i < 2; ++i )
    {
        for( int32_t j = 0; j < 2; ++j )
        {
            for( int32_t k = 0; k < 2; ++k )
            {
                // based on gluProject code from SGI implementation
                const Matrix4d& mv = frustum.getModelViewMatrix();
                const Matrix4d& proj = frustum.getProjectionMatrix();

                Vector4d in( x[ i ], y[ j ], z[ k ], 1.0 );
                Vector4d out = mv * in;
                in = proj * out;

                if( in.w() == 0.0 )
                    continue;
                in.normalize();

                /* The perspective correction is done in the constructor */
                Vector3d in3( in );

                /* Map x, y and z to range 0-1 */
                in3 = in3 * 0.5 + 0.5;

                /* Map x,y to viewport */
                in3[0] = in3[0] * pvp[2] + pvp[0];
                in3[1] = in3[1] * pvp[3] + pvp[1];

                if( in3[0] > xMax )
                    xMax = in3[0];
                if( in3[1] > yMax )
                    yMax = in3[1];

                if( in3[0] < xMin )
                    xMin = in3[0];
                if( in3[1] < yMin )
                    yMin = in3[1];
            }
        }
    }

    xMin = maths::clamp( xMin + 0.5, (double)pvp[0],
                                     (double)pvp[0] + (double)pvp[2] );
    yMin = maths::clamp( yMin + 0.5, (double)pvp[1],
                                     (double)pvp[1] + (double)pvp[3] );
    xMax = maths::clamp( xMax + 0.5, (double)pvp[0],
                                     (double)pvp[0] + (double)pvp[2] );
    yMax = maths::clamp( yMax + 0.5, (double)pvp[1],
                                     (double)pvp[1] + (double)pvp[3] );

    minScreenPos = Vector2i( xMin, yMin  );
    maxScreenPos = Vector2i( xMax, yMax  );
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

    const Boxf& worldBox = _lodNode->getWorldBox();
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
