/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <livre/core/data/NodeId.h>

namespace livre
{

NodeId::NodeId()
    : _id( INVALID_NODE_ID )
{}

NodeId::NodeId( const Identifier& identifier )
    : _id( identifier )
{}

NodeId::NodeId( const uint32_t level,
                const Vector3ui& position,
                const uint32_t frame )
    : _level( level ),
      _frame( frame )
{
    _blockPosX = position[ 0 ];
    _blockPosY = position[ 1 ];
    _blockPosZ = position[ 2 ];
}

Vector3ui NodeId::getPosition( ) const
{
    return Vector3ui( _blockPosX,
                      _blockPosY,
                      _blockPosZ  );
}

NodeIds NodeId::getParents( ) const
{
    NodeIds nodeIds;
    NodeId parent = getParent();

    while( parent.isValid() )
    {
        nodeIds.push_back( parent );
        parent = parent.getParent();
    }

    return nodeIds;
}

NodeId NodeId::getParent( ) const
{
    if( _level == INVALID_LEVEL || _level == 0 )
        return NodeId();

    const Vector3ui parentPos = getPosition() / 2;
    return NodeId( _level - 1, parentPos, _frame );
}

bool NodeId::isParent( const NodeId& parentNodeId ) const
{
    if( parentNodeId._level >= _level ||
        parentNodeId._frame != _frame ||
        parentNodeId._id == _id )
    {
        return false;
    }

    const uint32_t levelDiff = _level - parentNodeId._level;

    return ( ( _blockPosX << levelDiff ) == parentNodeId._blockPosX &&
             ( _blockPosY << levelDiff ) == parentNodeId._blockPosY &&
             ( _blockPosZ << levelDiff ) == parentNodeId._blockPosZ );
}

bool NodeId::isChild( const NodeId& childNodeId ) const
{
   return childNodeId.isParent( *this );
}

NodeIds NodeId::getChildren( ) const
{
    NodeIds nodeIds;
    if( _level == INVALID_LEVEL )
        return nodeIds;

    const Vector3ui childPos = getPosition() * 2;
    for( uint32_t x = 0; x < 2; ++x )
        for( uint32_t y = 0; y < 2; ++y )
            for( uint32_t z = 0; z < 2; ++z )
            {
                const Vector3ui pos( childPos[ 0 ] + x,
                                     childPos[ 1 ] + y,
                                     childPos[ 2 ] + z );
                nodeIds.push_back( NodeId( _level + 1, pos, _frame ) );
            }

    return nodeIds;
}

NodeId NodeId::getRoot() const
{
    return NodeId( 0, getPosition() / ( 1u << _level ), _frame );
}

NodeIds NodeId::getSiblings( ) const
{
    if( _level == INVALID_LEVEL || _level == 0 )
        return NodeIds();

    const NodeId& parentId = getParent();
    return parentId.getChildren();
}

NodeIds NodeId::getChildrenAtLevel( const uint32_t level ) const
{
    NodeIds nodeIds;
    if( _level == INVALID_LEVEL || _level >= level )
        return nodeIds;

    const uint32_t childCount = 1u << ( level - _level );
    const Vector3f& startPosInLevel = getPosition() * childCount;

    for( uint32_t x = 0; x < childCount; ++x )
        for( uint32_t y = 0; y < childCount; ++y )
            for( uint32_t z = 0; z < childCount; ++z )
            {
                const Vector3ui pos( startPosInLevel[ 0 ] + x,
                                     startPosInLevel[ 1 ] + y,
                                     startPosInLevel[ 2 ] + z );
                nodeIds.push_back( NodeId( level, pos, _frame ) );
            }

    return nodeIds;
}

}
