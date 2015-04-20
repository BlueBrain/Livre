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

#ifndef _livreEvents_h_
#define _livreEvents_h_

#include <livre/core/types.h>
#include <lunchbox/uint128_t.h>
#include <zeq/types.h>

namespace livre
{
namespace zeq
{
static const lunchbox::uint128_t EVENT_DATASOURCE(
    lunchbox::make_uint128( "livre::zeq::DataSourceEvent" ));

static const lunchbox::uint128_t EVENT_DATASOURCE_DATA(
    lunchbox::make_uint128( "livre::zeq::DataSourceDataEvent" ));

/** Serialize a data source event. */
::zeq::Event serializeDataSource( const lunchbox::URI& uri );

/** Deserialize a data source event. */
lunchbox::URI deserializeDataSource( const ::zeq::Event& event );


/**
 * Response to a data source event.
 *
 * The remote data source assigns each data source a unique event identifier,
 * which is used to request data on the initialized data source, i.e., the
 * provided uint128_t is the event to request data, and the uint128_t + 1 is the
 * event on which data is delivered.
 */
typedef std::pair< lunchbox::uint128_t,
                   livre::VolumeInformation > RemoteInformation;

/** Serialize information of a data source. */
::zeq::Event serializeDataSourceData( const RemoteInformation& );

/** Deserialize information of a data source. */
RemoteInformation deserializeDataSourceData( const ::zeq::Event& );


/**
 * Deserialized data sample event. See RemoteInformation for uint128_t semantic.
 */
typedef std::pair< lunchbox::uint128_t, LODNode > LODNodeSample;

/** Serialize the request for the data of one node. */
::zeq::Event serializeDataSample( const uint128_t& id,
                                  const livre::LODNode& node );

/** Deserialize the request for the data of one node. */
LODNodeSample deserializeDataSample( const ::zeq::Event& event );


/** Serialize the response for the data of one node. */
::zeq::Event serializeDataSampleData( const uint128_t& id,
                                      const MemoryUnitPtr data );

/** Deserialized data for one node. Valid until given event is disposed. */
typedef std::pair< const uint8_t*, size_t > LODNodeSampleData;

/** Deserialize the request for the data of one node. */
LODNodeSampleData deserializeDataSampleData( const ::zeq::Event& event );

}
}

#endif // _events_h_
