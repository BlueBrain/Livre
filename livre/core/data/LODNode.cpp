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

#include <livre/core/data/LODNode.h>
#include <livre/core/data/VolumeDataSource.h>

namespace livre
{

LODNode LODNode::empty = LODNode();

LODNode::LODNode( ) :
    blockSize_( 0 )
{}

LODNode::LODNode( const NodeId& nodeId,
                  const Vector3i& blockSize,
                  const Vector3i& levelTotalBlockDimensions )
   : nodeId_( nodeId )
   , blockSize_( blockSize )
{
    initialize_();
    computeWorldBox_( levelTotalBlockDimensions );
}

LODNode::LODNode( const NodeId& nodeId,
                  const Vector3i& blockSize,
                  const Boxf& worldBox )
   : nodeId_( nodeId )
   , blockSize_( blockSize )
   , worldBox_( worldBox )
{
    initialize_();
}

void LODNode::computeWorldBox_( const Vector3ui& levelTotalBlockDimensions )
{
    Vector3f lBoxCoordMin = getAbsolutePosition();
    Vector3f lBoxCoordMax( lBoxCoordMin + Vector3i( 1 ));
    const size_t index = levelTotalBlockDimensions.find_max_index();

    lBoxCoordMin = lBoxCoordMin / levelTotalBlockDimensions[index];
    lBoxCoordMax = lBoxCoordMax / levelTotalBlockDimensions[index];

    worldBox_ =  Boxf( lBoxCoordMin, lBoxCoordMax );
}

void LODNode::initialize_( )
{
    const Vector3i pntPos = getAbsolutePosition() * blockSize_;
    localVoxelBox_ = Boxui( pntPos, pntPos + blockSize_ );
}

Vector3f LODNode::getRelativePosition() const
{
    return Vector3f( getAbsolutePosition( )) / float( 1 << getRefLevel( ));
}

}
