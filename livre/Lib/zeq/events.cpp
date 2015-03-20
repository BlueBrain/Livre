/* Copyright (c) 2014, EPFL/Blue Brain Project
 *                     Stefan.Eilemann@epfl.ch
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

#include "events.h"

#include <livre/core/Data/LODNode.h>
#include <livre/core/Data/MemoryUnit.h>
#include <livre/core/Data/VolumeInformation.h>
#include <livre/Lib/lodNode_generated.h>
#include <livre/Lib/memory_generated.h>
#include <livre/Lib/uri_generated.h>
#include <livre/Lib/volumeInformation_generated.h>
#include <zeq/event.h>
#include <zeq/vocabulary.h>
#include <lunchbox/uri.h>
#include <boost/lexical_cast.hpp>

namespace livre
{
namespace zeq
{
::zeq::Event serializeDataSource( const lunchbox::URI& uri )
{
    ::zeq::Event event( EVENT_DATASOURCE );

    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    auto uriStr = fbb.CreateString( boost::lexical_cast< std::string >( uri ));
    FinishURIBuffer( fbb, CreateURI( fbb, uriStr ));

    return event;
}

lunchbox::URI deserializeDataSource( const ::zeq::Event& event )
{
    if( event.getType() != EVENT_DATASOURCE )
        return lunchbox::URI();

    auto data = GetURI( event.getData( ));
    return lunchbox::URI( data->uri()->c_str( ));
}

::zeq::Event serializeDataSourceData( const RemoteInformation& info )
{
    ::zeq::Event event( EVENT_DATASOURCE_DATA );
    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();

    // must construct nested structs before - only one fbb op can be in progress
    const livre::VolumeInformation& vi = info.second;
    auto overlap = fbb.CreateVector( vi.overlap.array, 3 );
    auto maximumBlockSize = fbb.CreateVector( vi.maximumBlockSize.array, 3 );
    auto minPos = fbb.CreateVector( vi.minPos.array, 3 );
    auto maxPos = fbb.CreateVector( vi.maxPos.array, 3 );
    auto voxels = fbb.CreateVector( vi.voxels.array, 3 );
    auto worldSize = fbb.CreateVector( vi.worldSize.array, 3 );
    auto bboxMin = fbb.CreateVector( vi.boundingBox.getMin().array, 3 );
    auto bboxMax = fbb.CreateVector( vi.boundingBox.getMax().array, 3 );

    VolumeInformationBuilder builder( fbb );
    builder.add_eventLow( info.first.low( ));
    builder.add_eventHigh( info.first.high( ));
    builder.add_isBigEndian( vi.isBigEndian );
    builder.add_compCount( vi.compCount );
    builder.add_depth( vi.rootNode.getDepth( ));
    builder.add_dataType( vi.dataType );
    builder.add_overlap( overlap );
    builder.add_maximumBlockSize( maximumBlockSize );
    builder.add_minPos( minPos );
    builder.add_maxPos( maxPos );
    builder.add_voxels( voxels );
    builder.add_worldSize( worldSize );
    builder.add_boundingBoxMin( bboxMin );
    builder.add_boundingBoxMax( bboxMax );
    builder.add_worldSpacePerVoxel( vi.worldSpacePerVoxel );

    fbb.Finish( builder.Finish( ));
    return event;
}

namespace
{
template< class T >
vmml::vector< 3, T > _deserializeVector3( const flatbuffers::Vector< T > *in )
{
    return vmml::vector< 3, T >( in->Get(0), in->Get(1), in->Get(2) );
}
}

RemoteInformation deserializeDataSourceData( const ::zeq::Event& event )
{
    if( event.getType() != EVENT_DATASOURCE_DATA )
        return RemoteInformation();

    auto data = GetVolumeInformation( event.getData( ));
    RemoteInformation info;
    livre::VolumeInformation& vi = info.second;



    info.first.low() = data->eventLow();
    info.first.high() = data->eventHigh();
    vi.isBigEndian = data->isBigEndian();
    vi.compCount = data->compCount();
    vi.dataType = DataType( data->dataType( ));
    vi.overlap = _deserializeVector3< unsigned >( data->overlap( ));
    vi.maximumBlockSize = _deserializeVector3< unsigned >(
                                 data->maximumBlockSize( ));
    vi.minPos = _deserializeVector3< float >( data->minPos( ));
    vi.maxPos = _deserializeVector3< float >( data->maxPos( ));
    vi.voxels = _deserializeVector3< unsigned >( data->voxels( ));
    vi.worldSize = _deserializeVector3< float >( data->worldSize( ));
    vi.boundingBox.getMin() = _deserializeVector3< float >(
                                     data->boundingBoxMin( ));
    vi.boundingBox.getMax() = _deserializeVector3< float >(
                                     data->boundingBoxMax( ));
    vi.worldSpacePerVoxel = data->worldSpacePerVoxel();

    const Vector3ui& blockSize = vi.maximumBlockSize - vi.overlap * 2;
    Vector3ui blocksSize = vi.voxels / blockSize;
    blocksSize = blocksSize / ( 1u << data->depth( ));

    vi.rootNode = RootNode( data->depth(), blocksSize );
    return info;
}

::zeq::Event serializeDataSample( const uint128_t& id,
                                  const livre::LODNode& node )
{
    ::zeq::Event event( id );
    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();

    // must construct nested structs before - only one fbb op can be in progress
    auto blockSize = fbb.CreateVector( node.getBlockSize().array, 3 );
    auto worldBoxMin = fbb.CreateVector( node.getWorldBox().getMin().array, 3 );
    auto worldBoxMax = fbb.CreateVector( node.getWorldBox().getMax().array, 3 );

    LODNodeBuilder builder( fbb );
    builder.add_nodeId( node.getNodeId().getId( ));
    builder.add_maxRefLevel( node.getMaxRefLevel( ));
    builder.add_blockSize( blockSize );
    builder.add_worldBoxMin( worldBoxMin );
    builder.add_worldBoxMax( worldBoxMax );

    fbb.Finish( builder.Finish( ));
    return event;
}

LODNodeSample deserializeDataSample( const ::zeq::Event& event )
{
    LODNodeSample sample;
    sample.first = event.getType();

    auto data = GetLODNode( event.getData( ));
    sample.second = livre::LODNode(
        NodeId( data->nodeId( )),
        data->maxRefLevel(),
        _deserializeVector3< int32_t >( data->blockSize( )),
        Boxf( _deserializeVector3< float >( data->worldBoxMin( )),
              _deserializeVector3< float >( data->worldBoxMax( ))));

    return sample;
}

::zeq::Event serializeDataSampleData( const uint128_t& id,
                                      const MemoryUnitPtr memory )
{
    ::zeq::Event event( id );
    flatbuffers::FlatBufferBuilder& fbb = event.getFBB();
    const size_t size = memory->getMemSize();
    auto data = fbb.CreateUninitializedVector< uint8_t >( size );

    // https://groups.google.com/d/msg/flatbuffers/GWsYUlTYsQs/CCzihVQqpYcJ
    uint8_t* ptr = const_cast< uint8_t* >(
        reinterpret_cast< flatbuffers::Vector< const uint8_t >* >(
            fbb.GetBufferPointer( ))->Data( ));
    ::memcpy( ptr, memory->getData< uint8_t >(), size );

    MemoryBuilder builder( fbb );
    builder.add_data( data );

    fbb.Finish( builder.Finish( ));
    return event;
}

LODNodeSampleData deserializeDataSampleData( const ::zeq::Event& event )
{
    auto data = GetMemory( event.getData( ))->data();
    return std::make_pair( data->Data(), data->size( ));
}

}
}
