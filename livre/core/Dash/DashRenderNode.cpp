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
    DNT_TEXTURE_REQUESTED ,
    DNT_DATA_REQUESTED    ,
    DNT_CACHE_MODIFIED    ,

    // Specific to root tree node
    DNT_TREE_PRIORITY     ,
    DNT_FRAME_ID          ,
    DNT_THREAD_OPERATION  ,
    DNT_FRUSTUM
};

DashRenderNode::DashRenderNode( dash::NodePtr dashNode )
    : dashNode_( dashNode )
{ }

DashRenderNode DashRenderNode::getParent( ) const
{
    dash::NodePtr parentNode;
    if( dashNode_->getNParents() == 1 )
        parentNode = dashNode_->getParent( 0 );
    return DashRenderNode( parentNode );
}

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

bool DashRenderNode::isDataRequested( ) const
{
    return getAttribute_< bool >( DNT_DATA_REQUESTED );
}

bool DashRenderNode::isTextureRequested( ) const
{
    return getAttribute_< bool >( DNT_TEXTURE_REQUESTED );
}

void DashRenderNode::setLODNode( const LODNode& node )
{
    *(dashNode_->getAttribute( DNT_NODE )) = node;
}

void DashRenderNode::setTextureDataObject( ConstCacheObjectPtr textureData )
{
    *(dashNode_->getAttribute( DNT_TEXTUREDATA )) = textureData;
}

void DashRenderNode::setTextureObject( ConstCacheObjectPtr texture )
{
    *(dashNode_->getAttribute( DNT_TEXTURE )) = texture;
}

void DashRenderNode::setVisible( bool visibility )
{
    *(dashNode_->getAttribute( DNT_VISIBLE )) = visibility;
}

void DashRenderNode::setDataRequested( bool isRequested )
{
    *(dashNode_->getAttribute( DNT_DATA_REQUESTED )) = isRequested;
}

void DashRenderNode::setTextureRequested( bool isRequested )
{
    *(dashNode_->getAttribute( DNT_TEXTURE_REQUESTED )) = isRequested;
}

void DashRenderNode::setRequested( RequestType requestType, bool isRequested )
{
    switch( requestType )
    {
        case DRT_VISIBILE:
            setVisible( isRequested );
            break;
        case DRT_DATA:
            setDataRequested( isRequested );
            break;
        case DRT_TEXTURE:
            setTextureRequested( isRequested );
            break;
    }
}

bool DashRenderNode::hasParent( ) const
{
    return dashNode_->hasParents();
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

    dash::AttributePtr isTextureRequested = new dash::Attribute();
    *isTextureRequested = false;
    dashNode->insert( isTextureRequested );

    dash::AttributePtr isDataRequested = new dash::Attribute();
    *isDataRequested = false;
    dashNode->insert( isDataRequested );

    dash::AttributePtr cacheObjectModified = new dash::Attribute();
    *cacheObjectModified = true;
    dashNode->insert( cacheObjectModified );
}

void DashRenderNode::initializeRootNode( dash::NodePtr dashNode )
{
    initializeDashNode( dashNode );

    dash::AttributePtr priority = new dash::Attribute();
    *priority = LP_NONE;
    dashNode->insert( priority );

    dash::AttributePtr currentRenderID = new dash::Attribute();
    *currentRenderID = (uint64_t)0;
    dashNode->insert( currentRenderID );

    dash::AttributePtr threadOp = new dash::Attribute();
    *threadOp = TO_NONE;
    dashNode->insert( threadOp );

    dash::AttributePtr frustum = new dash::Attribute();
    *frustum = Frustum();
    dashNode->insert( frustum );
}

LoadPriority DashRenderNode::rootGetLoadPriority_( ) const
{
    return getAttribute_< LoadPriority >( DNT_TREE_PRIORITY );
}

void DashRenderNode::rootSetTreePriority_( LoadPriority priority )
{
    *(dashNode_->getAttribute( DNT_TREE_PRIORITY )) = priority;
}

uint64_t DashRenderNode::rootGetFrameID_( ) const
{
    return getAttribute_< uint64_t >( DNT_FRAME_ID );
}

void DashRenderNode::rootSetFrameID_( const uint64_t frameId )
{
    *(dashNode_->getAttribute( DNT_FRAME_ID )) = frameId;
}

Frustum DashRenderNode::rootGetFrustum_( ) const
{
    return getAttribute_< Frustum >( DNT_FRUSTUM );
}

void DashRenderNode::rootSetFrustum_( const Frustum& frustum )
{
    *(dashNode_->getAttribute( DNT_FRUSTUM )) = frustum;
}

ThreadOperation DashRenderNode::rootGetThreadOp_( ) const
{
    return getAttribute_< ThreadOperation >( DNT_THREAD_OPERATION );
}

void DashRenderNode::rootSetThreadOp_( const ThreadOperation op )
{
    *(dashNode_->getAttribute( DNT_THREAD_OPERATION )) = op;
}

template< class T >
T DashRenderNode::getAttribute_( const uint32_t nodeType ) const
{
    dash::ConstAttributePtr attribute = dashNode_->getAttribute( nodeType );
    return attribute->getUnsafe< T >( );
}


}
