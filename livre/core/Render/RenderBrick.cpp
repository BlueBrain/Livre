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

#include "RenderBrick.h"
#include "gl.h"

#include <livre/core/Render/Frustum.h>
#include <livre/core/Data/LODNode.h>
#include <livre/core/Render/Viewport.h>

#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated" // gluProject
#endif

namespace livre
{


RenderBrick::RenderBrick( ConstLODNodePtr lodNodePtr, ConstTextureStatePtr textureState )
    : LODNodeTrait( lodNodePtr ),
      textureState_( textureState )
{
}

ConstTextureStatePtr RenderBrick::getTextureState() const
{
    return textureState_;
}

void RenderBrick::getScreenCoordinates( const Frustum& frustum,
                                        const Viewporti& pixelViewPort,
                                        Vector2i& minScreenPos,
                                        Vector2i& maxScreenPos ) const
{
    const Vector3f& minPos = lodNodePtr_->getWorldBox().getMin();
    const Vector3f& maxPos = lodNodePtr_->getWorldBox().getMax();

    const double x[ 2 ] = { minPos[ 0 ], maxPos[ 0 ] };
    const double y[ 2 ] = { minPos[ 1 ], maxPos[ 1 ] };
    const double z[ 2 ] = { minPos[ 2 ], maxPos[ 2 ] };

    double xMax = -std::numeric_limits< double >::max();
    double yMax = -std::numeric_limits< double >::max();

    double xMin = -xMax;
    double yMin = -yMax;

    Vector4i pvp;
    pvp[ 0 ] = pixelViewPort.getPosition()[ 0 ];
    pvp[ 1 ] = pixelViewPort.getPosition()[ 1 ];
    pvp[ 2 ] = pixelViewPort.getSize()[ 0 ];
    pvp[ 3 ] = pixelViewPort.getSize()[ 1 ];

    for( int32_t i = 0; i < 2; ++i )
    {
        for( int32_t j = 0; j < 2; ++j )
        {
            for( int32_t k = 0; k < 2; ++k )
            {
                double xProj, yProj, zProj;
                const Matrix4d mv = frustum.getModelViewMatrix();
                const Matrix4d proj = frustum.getProjectionMatrix();
                gluProject( x[ i ], y[ j ], z[ k ],
                            mv.array,
                            proj.array,
                            pvp.array,
                            &xProj, &yProj, &zProj );

                if( xProj > xMax )
                    xMax = xProj;
                if( yProj > yMax )
                    yMax = yProj;

                if( xProj < xMin )
                    xMin = xProj;
                if( yProj < yMin )
                    yMin = yProj;
            }
        }
    }

    xMin = std::max( (int)floor( xMin + 0.5 ), pvp[ 0 ] );
    yMin = std::max( (int)floor( yMin + 0.5 ), pvp[ 1 ] );

    xMax = std::min( (int)floor( xMax + 0.5 ), pvp[ 2 ] );
    yMax = std::min( (int)floor( yMax + 0.5 ), pvp[ 3 ] );

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

    const Boxf& worldBox = lodNodePtr_->getWorldBox();
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
