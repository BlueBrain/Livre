/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <livre/lib/render/RenderView.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/render/Frustum.h>
#include <livre/core/dash/DashRenderNode.h>
#include <livre/core/dash/DashTree.h>
#include <livre/core/render/RenderingSetGenerator.h>
#include <livre/core/maths/maths.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/data/VolumeInformation.h>
#include <livre/core/render/GLWidget.h>

namespace livre
{

RenderView::RenderView( )
{
}

void RenderView::setParameters( ConstVolumeRendererParametersPtr vrParams )
{
    volumeRendererParameters_ = *vrParams;
}

void RenderView::onPostRender_( const GLWidget&,
                                const FrameInfo& frameInfo )
{
    freeTextures_( frameInfo.renderNodeList );
}

void RenderView::freeTextures_( const DashNodeVector& renderNodeList )
{
    DashNodeSet currentSet;
    for( DashNodeVector::const_iterator it = renderNodeList.begin();
         it != renderNodeList.end(); ++it )
    {
        dash::NodePtr node = *it;
        currentSet.insert( node );
    }

    DashNodeSet cleanSet;
    std::set_difference( previousVisibleSet_.begin(),
                         previousVisibleSet_.end(),
                         currentSet.begin(),
                         currentSet.end(),
                         std::inserter( cleanSet, cleanSet.begin( )));

    // Unreference not needed textures
    for( DashNodeSet::iterator it = cleanSet.begin(); it != cleanSet.end(); ++it )
    {
        DashRenderNode renderNode( *it );
        if( renderNode.getLODNode().getRefLevel() != 0 )
            renderNode.setTextureObject( TextureObject::getEmptyPtr( ));
    }

    previousVisibleSet_ = currentSet;
}
}
