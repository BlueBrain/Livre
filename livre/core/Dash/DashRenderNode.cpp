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

#include <livre/core/Dash/DashRenderNode.h>

namespace livre
{

enum DashAttributeType
{
    DNT_NODE              ,
    DNT_TEXTUREDATA       ,
    DNT_TEXTURE           ,
    DNT_VISIBLE           ,
    DNT_CACHE_MODIFIED
};

DashRenderNode::DashRenderNode( dash::NodePtr dashNode )
    : _dashNode( dashNode )
{ }

const LODNode& DashRenderNode::getLODNode( ) const
{
    return getAttribute_< const LODNode& >( DashAttributeType::DNT_NODE );
}

ConstCacheObjectPtr DashRenderNode::getTextureDataObject( ) const
{
    return getAttribute_< ConstCacheObjectPtr >( DNT_TEXTUREDATA );
}

ConstCacheObjectPtr DashRenderNode::getTextureObject( ) const
{
    return getAttribute_< ConstCacheObjectPtr >( DNT_TEXTURE );
}

bool DashRenderNode::isVisible( ) const
{
    return getAttribute_< bool >( DNT_VISIBLE );
}

void DashRenderNode::setLODNode( const LODNode& node )
{
    *(_dashNode->getAttribute( DNT_NODE )) = node;
}

void DashRenderNode::setTextureDataObject( ConstCacheObjectPtr textureData )
{
    *(_dashNode->getAttribute( DNT_TEXTUREDATA )) = textureData;
}

void DashRenderNode::setTextureObject( ConstCacheObjectPtr texture )
{
    *(_dashNode->getAttribute( DNT_TEXTURE )) = texture;
}

void DashRenderNode::setVisible( bool visibility )
{
    *(_dashNode->getAttribute( DNT_VISIBLE )) = visibility;
}

void DashRenderNode::initializeDashNode( dash::NodePtr dashNode )
{
    dash::AttributePtr node = new dash::Attribute();
    const LODNode lodNode;
    *node = lodNode;
    dashNode->insert( node );

    dash::AttributePtr textureData = new dash::Attribute();
    ConstCacheObjectPtr lodTextureData( EmptyCacheObject::getEmptyPtr() );
    *textureData = lodTextureData;
    dashNode->insert( textureData );

    dash::AttributePtr texture = new dash::Attribute();
    ConstCacheObjectPtr lodTexture( EmptyCacheObject::getEmptyPtr() );
    *texture = lodTexture;
    dashNode->insert( texture );

    dash::AttributePtr isVisible = new dash::Attribute();
    *isVisible = false;
    dashNode->insert( isVisible );

    dash::AttributePtr cacheObjectModified = new dash::Attribute();
    *cacheObjectModified = true;
    dashNode->insert( cacheObjectModified );
}

template< class T >
T DashRenderNode::getAttribute_( const uint32_t nodeType ) const
{
    dash::ConstAttributePtr attribute = _dashNode->getAttribute( nodeType );
    return attribute->getUnsafe< T >( );
}


}
