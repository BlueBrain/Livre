
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

#include "MemoryDataSource.h"

#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/version.h>

#include <lunchbox/pluginRegisterer.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

extern "C"
int LunchboxPluginGetVersion() { return LIVRECORE_VERSION_ABI; }

extern "C"
bool LunchboxPluginRegister() { return true; }

namespace livre
{

namespace
{
   lunchbox::PluginRegisterer< MemoryDataSource > registerer;
}

template<typename T>
MemoryUnitPtr computeData( const LODNode& node,
                           const size_t dataSize,
                           const float sparsity,
                           const Vector3ui& blockSize )
{
    const Identifier nodeId = node.getNodeId().getId();
    const uint8_t* id = reinterpret_cast< const uint8_t* >( &nodeId );
    const T value =  ( id[0] ^ id[1] ^ id[2] ^ id[3] ) + 16 +
        127 * std::sin( ((float)node.getNodeId().getTimeStep() + 1) / 200.f);

    AllocMemoryUnitPtr memoryUnit( new AllocMemoryUnit( dataSize ));
    T* dstData = memoryUnit->getData< T >();
    for( size_t i = 0; i < blockSize.product(); ++i )
    {
        if( sparsity < 1.f )
        {
            const int32_t random = rand() % 1000000 + 1;
            dstData[ i ] =  random < 1000000.0f * sparsity ? value : 0;
        }
        else
            dstData[ i ] = value ;
    }
    return memoryUnit;
}

MemoryDataSource::MemoryDataSource( const DataSourcePluginData& initData )
{
    _volumeInfo.overlap = Vector3ui( 4 );

    const servus::URI& uri = initData.getURI();
    std::vector< std::string > parameters;
    boost::algorithm::split( parameters, uri.getFragment(),
                             boost::is_any_of( "," ));

    using boost::lexical_cast;
    try
    {
        servus::URI::ConstKVIter i = uri.findQuery( "sparsity" );
        _sparsity = i == uri.queryEnd() ? 1.0f : lexical_cast< float >( i->second );

        i = uri.findQuery( "datatype" );
        if( i == uri.queryEnd() || i->second == "uint8" )
            _volumeInfo.dataType = DT_UINT8;
        else if( i->second == "uint16" )
            _volumeInfo.dataType = DT_UINT16;
        else if( i->second == "uint32" )
            _volumeInfo.dataType = DT_UINT32;
        else if( i->second == "int8" || i->second == "char" )
            _volumeInfo.dataType = DT_INT8;
        else if( i->second == "int16" || i->second == "short" )
            _volumeInfo.dataType = DT_INT16;
        else if( i->second == "int32" )
            _volumeInfo.dataType = DT_INT32;
        else if( i->second == "float" )
            _volumeInfo.dataType = DT_FLOAT;
    }
    catch( boost::bad_lexical_cast& except )
        LBTHROW( std::runtime_error( except.what( )));

    if( parameters.size() < 4 ) // use defaults
    {
        _volumeInfo.voxels = Vector3ui( 4096 );
        _volumeInfo.maximumBlockSize = Vector3ui(32) + _volumeInfo.overlap * 2;
    }
    else
    {
        try
        {
            _volumeInfo.voxels[ 0 ] = lexical_cast< uint32_t >( parameters[0] );
            _volumeInfo.voxels[ 1 ] = lexical_cast< uint32_t >( parameters[1] );
            _volumeInfo.voxels[ 2 ] = lexical_cast< uint32_t >( parameters[2] );
            const Vector3ui blockSize( lexical_cast< uint32_t >(parameters[3]));
            _volumeInfo.maximumBlockSize = blockSize + _volumeInfo.overlap * 2;
        }
        catch( boost::bad_lexical_cast& except )
            LBTHROW( std::runtime_error( except.what() ));
    }

    _volumeInfo.frameRange = FULL_FRAME_RANGE;

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

    switch( _volumeInfo.dataType )
    {
        case DT_UINT8:
            return computeData< uint8_t >( node, dataSize, _sparsity, blockSize );
        case DT_UINT16:
            return computeData< uint16_t >( node, dataSize, _sparsity, blockSize );
        case DT_UINT32:
            return computeData< uint32_t >( node, dataSize, _sparsity, blockSize );
        case DT_INT8:
            return computeData< int8_t >( node, dataSize, _sparsity, blockSize );
        case DT_INT16:
            return computeData< int16_t >( node, dataSize, _sparsity, blockSize );
        case DT_INT32:
            return computeData< int32_t >( node, dataSize, _sparsity, blockSize );
        case DT_FLOAT:
            return computeData< float >( node, dataSize, _sparsity, blockSize );
        default:
            LBTHROW( std::runtime_error( "Unimplemented data type." ));
    }
}

bool MemoryDataSource::handles( const DataSourcePluginData& initData )
{
    return initData.getURI().getScheme() == "mem";
}

}
