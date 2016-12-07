
/* Copyright (c) 2016, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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


#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/lib/data/RawDataSource.h>

#include <lunchbox/memoryMap.h>
#include <lunchbox/pluginRegisterer.h>

#include "nrrd/nrrd.hxx"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace livre
{
namespace
{
   lunchbox::PluginRegisterer< RawDataSource > registerer;
}

using boost::lexical_cast;

struct RawDataSource::Impl
{
    Impl( const DataSourcePluginData& initData, VolumeInformation& volInfo )
        : _headerSize( 0 )
    {
        const servus::URI& uri = initData.getURI();
        const std::string& path = uri.getPath();
        const bool isExtensionRaw =
                boost::algorithm::ends_with( path, ".raw" ) ||
                boost::algorithm::ends_with( path, ".img" );
        const bool isExtensionNrrd = boost::algorithm::ends_with( path, ".nrrd" );

        if( !isExtensionRaw && !isExtensionNrrd )
            LBTHROW( std::runtime_error( "Volume extension does not include raw or nrrd" ));

        if( isExtensionRaw )
            parseRawData( uri.getPath(), volInfo, uri.getFragment( ));
        else if( isExtensionNrrd )
            parseNRRDData( uri.getPath(), volInfo );

        volInfo.frameRange = Vector2ui( 0u, 1u );
        volInfo.compCount = 1;
        volInfo.worldSpacePerVoxel = 1.0f / float( volInfo.voxels.find_max( ));
        volInfo.worldSize =
            Vector3f( volInfo.voxels[0], volInfo.voxels[1], volInfo.voxels[2] )*
            volInfo.worldSpacePerVoxel;

        volInfo.overlap = Vector3ui( 0u );
        volInfo.rootNode = RootNode( 1, Vector3ui( 1 ));
        volInfo.maximumBlockSize = volInfo.voxels;
    }

    ~Impl() {}

    MemoryUnitPtr getData( const LODNode& node )
    {
        const size_t dataSize = node.getBlockSize().product();
        return MemoryUnitPtr(
            new ConstMemoryUnit( _mmap.getAddress< uint8_t >() + _headerSize,
                                 dataSize ));
    }

    DataType getDataType( const std::string& dataType )
    {
        if( dataType == "char" || dataType == "int8" )
            return DT_INT8;
        if( dataType == "unsigned char" || dataType == "uint8" )
            return DT_UINT8;
         if( dataType == "short" || dataType == "int16" )
             return DT_INT16;
         if( dataType == "unsigned short" || dataType == "uint16" )
             return DT_UINT16;
         if( dataType == "int" || dataType == "int32" )
             return DT_INT32;
         if( dataType == "unsigned int" || dataType == "uint32" )
             return DT_UINT32;
         if( dataType == "float" )
             return DT_FLOAT;
         LBTHROW( std::runtime_error( "Not supported data format" ));
    }

    void parseRawData( const std::string& filename, VolumeInformation& volInfo,
                       const std::string& fragment)
    {
        if( !_mmap.map( filename ))
            LBTHROW( std::runtime_error( "Cannot mmap file" ));

        std::vector< std::string > parameters;
        boost::algorithm::split( parameters, fragment, boost::is_any_of( "," ));

        if( parameters.size() < 4 ) // use defaults
        {
            LBTHROW( std::runtime_error( "Not enough parameters for the raw file" ));
        }
        else
        {
            try
            {
                volInfo.voxels[ 0 ] = lexical_cast< uint32_t >( parameters[0] );
                volInfo.voxels[ 1 ] = lexical_cast< uint32_t >( parameters[1] );
                volInfo.voxels[ 2 ] = lexical_cast< uint32_t >( parameters[2] );
                volInfo.dataType = getDataType( parameters[ 3 ] );
            }
            catch( boost::bad_lexical_cast& except )
            {
                LBTHROW( std::runtime_error( except.what() ));
            }
        }
   }

    void parseNRRDData( const std::string& filename,
                        VolumeInformation& volInfo )
    {
        std::map< std::string, std::string > dataInfo;
        _headerSize = ::NRRD::parseHeader( filename, dataInfo );
        if( _headerSize == 0 )
            LBTHROW( std::runtime_error( "Cannot parse nrrd file" ));

        std::string dataFile = filename;
        if( dataInfo.count( "datafile" ) > 0 )
        {
            boost::filesystem::path dataFilePath = boost::filesystem::path( filename ).parent_path();
            dataFilePath /= dataInfo[ "datafile" ];
            dataFile = dataFilePath.string();
        }

        if( !_mmap.map( dataFile ))
            LBTHROW( std::runtime_error( "Cannot mmap file" ));

        volInfo.dataType = getDataType( dataInfo["type"] );

        try
        {
            if( lexical_cast< size_t >( dataInfo[ "dimension" ] ) != 3u )
                LBTHROW( std::runtime_error( std::runtime_error( "NRRD is not 3D data" )))
        }
        catch( boost::bad_lexical_cast& except )
        {
            LBTHROW( std::runtime_error( except.what() ));
        }

        const auto& vec = stringToVector< int >( dataInfo[ "sizes" ] );
        volInfo.voxels[ 0 ] = vec[ 0 ];
        volInfo.voxels[ 1 ] = vec[ 1 ];
        volInfo.voxels[ 2 ] = vec[ 2 ];
        volInfo.bigEndian = dataInfo[ "endian" ] == "big";
    }

    lunchbox::MemoryMap _mmap;
    size_t _headerSize;
};

RawDataSource::RawDataSource( const DataSourcePluginData& initData )
    : _impl( new RawDataSource::Impl( initData, _volumeInfo ))
{}

RawDataSource::~RawDataSource()
{}

MemoryUnitPtr RawDataSource::getData( const LODNode& node )
{
    return _impl->getData( node );
}

bool RawDataSource::handles( const DataSourcePluginData& initData )
{
    const servus::URI& uri = initData.getURI();
    if( uri.getScheme() == "raw" )
        return true;

    if( !uri.getScheme().empty( ))
        return false;

    return boost::algorithm::ends_with( uri.getPath(), ".raw" ) ||
           boost::algorithm::ends_with( uri.getPath(), ".img" ) ||
           boost::algorithm::ends_with( uri.getPath(), ".nrrd" );
}

}
