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

#include <livre/core/Dash/DashRenderStatus.h>

namespace livre
{

enum DashAttributeType
{
    DNT_TREE_PRIORITY     ,
    DNT_FRAME_ID          ,
    DNT_THREAD_OPERATION  ,
    DNT_FRUSTUM
};

DashRenderStatus::DashRenderStatus()
    : _dashNode( new dash::Node() )
{
    dash::AttributePtr priority = new dash::Attribute();
    *priority = LP_NONE;
    _dashNode->insert( priority );

    dash::AttributePtr currentRenderID = new dash::Attribute();
    *currentRenderID = (uint64_t)0;
    _dashNode->insert( currentRenderID );

    dash::AttributePtr threadOp = new dash::Attribute();
    *threadOp = TO_NONE;
    _dashNode->insert( threadOp );

    dash::AttributePtr frustum = new dash::Attribute();
    *frustum = Frustum();
    _dashNode->insert( frustum );
}

LoadPriority DashRenderStatus::getLoadPriority( ) const
{
    return _getAttribute< LoadPriority >( DNT_TREE_PRIORITY );
}

void DashRenderStatus::setLoadPriority( const LoadPriority priority )
{
    *(_dashNode->getAttribute( DNT_TREE_PRIORITY )) = priority;
}

uint64_t DashRenderStatus::getFrameID( ) const
{
    return _getAttribute< uint64_t >( DNT_FRAME_ID );
}

void DashRenderStatus::setFrameID( const uint64_t frameId )
{
    *(_dashNode->getAttribute( DNT_FRAME_ID )) = frameId;
}

Frustum DashRenderStatus::getFrustum( ) const
{
    return _getAttribute< Frustum >( DNT_FRUSTUM );
}

void DashRenderStatus::setFrustum( const Frustum& frustum )
{
    *(_dashNode->getAttribute( DNT_FRUSTUM )) = frustum;
}

ThreadOperation DashRenderStatus::getThreadOp( ) const
{
    return _getAttribute< ThreadOperation >( DNT_THREAD_OPERATION );
}

void DashRenderStatus::setThreadOp( const ThreadOperation op )
{
    *(_dashNode->getAttribute( DNT_THREAD_OPERATION )) = op;
}

template< class T >
T DashRenderStatus::_getAttribute( const uint32_t nodeType ) const
{
    dash::ConstAttributePtr attribute = _dashNode->getAttribute( nodeType );
    return attribute->getUnsafe< T >( );
}


}
