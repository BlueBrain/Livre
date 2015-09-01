
/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/dash/DashRenderNode.h>

#include <livre/lib/data/MemoryDataSource.h>

namespace livre
{

namespace
{
   lunchbox::PluginRegisterer< MemoryDataSource > registerer;
}

MemoryDataSource::MemoryDataSource( const VolumeDataSourcePluginData& initData )
{
    _volumeInfo.overlap = Vector3ui( 4 );

    std::vector< std::string > parameters;
    boost::algorithm::split( parameters, initData.getURI().getFragment(),
                             boost::is_any_of( "," ));
    if( parameters.size() < 4 ) // use defaults
    {
        _volumeInfo.voxels = Vector3ui( 4096 );
        _volumeInfo.maximumBlockSize = Vector3ui(32) + _volumeInfo.overlap * 2;
    }
    else
    {
        try
        {
            using boost::lexical_cast;
            _volumeInfo.voxels[ 0 ] = lexical_cast< uint32_t >( parameters[0] );
            _volumeInfo.voxels[ 1 ] = lexical_cast< uint32_t >( parameters[1] );
            _volumeInfo.voxels[ 2 ] = lexical_cast< uint32_t >( parameters[2] );
            const Vector3ui blockSize( lexical_cast< uint32_t >(parameters[3]));
            _volumeInfo.maximumBlockSize = blockSize + _volumeInfo.overlap * 2;
        }
        catch( boost::bad_lexical_cast& except )
            LBTHROW( std::runtime_error( except.what() ));
    }

    if(!fillRegularVolumeInfo( _volumeInfo  ))
        LBTHROW( std::runtime_error( "Cannot setup the regular tree" ));
}

MemoryDataSource::~MemoryDataSource()
{
}

MemoryUnitPtr MemoryDataSource::getData( const LODNode& node )
{
    const Vector3i blockSize = node.getBlockSize() + _volumeInfo.overlap * 2;
    const size_t dataSize = blockSize.product() * _volumeInfo.compCount *
                            _volumeInfo.getBytesPerVoxel();
    const Identifier nodeID = node.getNodeId().getId();
    const uint8_t* id = reinterpret_cast< const uint8_t* >( &nodeID );
    const uint8_t fillValue = ( id[0] ^ id[1] ^ id[2] ^ id[3] ) + 16 +
        // emulate animation
        127 * std::sin( ((float)node.getNodeId().getFrame() + 1) / 200.f);
    AllocMemoryUnitPtr memoryUnit( new AllocMemoryUnit );
    memoryUnit->alloc( 1, dataSize );
    ::memset( memoryUnit->getData< uint8_t >(), fillValue, dataSize );
    return memoryUnit;
}

bool MemoryDataSource::handles( const VolumeDataSourcePluginData& initData )
{
    return initData.getURI().getScheme() == "mem";
}

livre::Vector2ui livre::MemoryDataSource::getFrameRange()
{
    return Vector2ui( 0, 1u << NODEID_FRAME_BITS );
}

}
