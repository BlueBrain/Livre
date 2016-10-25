
/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#include "RawDataSource.h"

#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/version.h>

#include <lunchbox/pluginRegisterer.h>

#include "nrrd/nrrd.hxx"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

extern "C"
int LunchboxPluginGetVersion() { return LIVRECORE_VERSION_ABI; }

extern "C"
bool LunchboxPluginRegister() { return true; }

namespace livre
{

namespace
{
   lunchbox::PluginRegisterer< RawDataSource > registerer;
}

struct RawDataSource::Impl
{
    Impl( const DataSourcePluginData& initData, VolumeInformation& volInfo )
        : _volInfo( volInfo )
        , _mmapPtr( nullptr )
        , _fd( -1 )
        , _rawDataSize( 0 )
        , _headerSize( 0 )
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
            parseRawData( uri.getPath(), uri.getFragment( ));
        else if( isExtensionNrrd )
            parseNRRDData( uri.getPath( ));

        _volInfo.frameRange = Vector2ui( 0u, 1u );
        _volInfo.compCount = 1;
        _volInfo.worldSpacePerVoxel = 1.0f / float( _volInfo.voxels.find_max( ));
        _volInfo.worldSize = Vector3f( _volInfo.voxels[0],
                                       _volInfo.voxels[1],
                                       _volInfo.voxels[2] ) * _volInfo.worldSpacePerVoxel;

        _volInfo.overlap = Vector3ui( 0u );
        _volInfo.rootNode = RootNode( 1, Vector3ui( 1 ));
        _volInfo.maximumBlockSize =  _volInfo.voxels;
    }

    ~Impl()
    {
        if( _mmapPtr != nullptr )
            ::munmap((void *)_mmapPtr, _rawDataSize + _headerSize );

        if( _fd != -1 )
            close( _fd );
    }

    bool memoryMap( const std::string& filename, const size_t headerSize = 0 )
    {
        _fd = open( filename.c_str(), O_RDONLY );
        if( _fd == -1 )
            return false;

        struct stat sb;
        if( ::fstat( _fd, &sb ) == -1 )
            return false;

        _rawDataSize = sb.st_size - headerSize;

        _mmapPtr = mmap( 0, sb.st_size, PROT_READ, MAP_PRIVATE, _fd, 0 );
        if( _mmapPtr == MAP_FAILED )
        {
            ::close( _fd );
            _fd = -1;
            _mmapPtr = nullptr;
            return false;
        }

        return true;
    }

    MemoryUnitPtr getData( const LODNode& node )
    {
        const size_t dataSize = node.getBlockSize().product();
        MemoryUnitPtr memUnitPtr( new ConstMemoryUnit( (const uint8_t*)_mmapPtr + _headerSize,
                                                       dataSize ));
        return memUnitPtr;
    }

    void setDataType( const std::string& dataType )
    {
        if( dataType == "char" || dataType == "int8" )
            _volInfo.dataType = DT_INT8;
        else if( dataType == "unsigned char" || dataType == "uint8" )
            _volInfo.dataType = DT_UINT8;
        else if( dataType == "short" || dataType == "int16" )
            _volInfo.dataType = DT_INT16;
        else if( dataType == "unsigned short" || dataType == "uint16" )
            _volInfo.dataType = DT_UINT16;
        else if( dataType == "int" || dataType == "int32" )
            _volInfo.dataType = DT_INT32;
        else if( dataType == "unsigned int" || dataType == "uint32" )
            _volInfo.dataType = DT_UINT32;
        else if( dataType == "float" )
            _volInfo.dataType = DT_FLOAT;
        else
            LBTHROW( std::runtime_error( "Not supported data format" ));
    }

    void parseRawData( const std::string& filename,
                       const std::string& fragment )
    {
        if( !memoryMap( filename ))
            LBTHROW( std::runtime_error( "Cannot mmap file" ));

        std::vector< std::string > parameters;
        boost::algorithm::split( parameters, fragment, boost::is_any_of( "," ));

        using boost::lexical_cast;
        if( parameters.size() < 4 ) // use defaults
        {
            LBTHROW( std::runtime_error( "Not enough parameters for the raw file" ));
        }
        else
        {
            try
            {
                _volInfo.voxels[ 0 ] = boost::lexical_cast< uint32_t >( parameters[0] );
                _volInfo.voxels[ 1 ] = boost::lexical_cast< uint32_t >( parameters[1] );
                _volInfo.voxels[ 2 ] = boost::lexical_cast< uint32_t >( parameters[2] );
                setDataType( parameters[ 3 ] );
            }
            catch( boost::bad_lexical_cast& except )
            {
                LBTHROW( std::runtime_error( except.what() ));
            }
        }
   }

    void parseNRRDData( const std::string& filename )
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

        if( !memoryMap( dataFile, _headerSize ))
            LBTHROW( std::runtime_error( "Cannot mmap file" ));

        setDataType( dataInfo["type"] );

        try
        {
            if( boost::lexical_cast< size_t >( dataInfo[ "dimension" ] ) != 3u )
                LBTHROW( std::runtime_error( std::runtime_error( "NRRD is not 3D data" )))
        }
        catch( boost::bad_lexical_cast& except )
        {
            LBTHROW( std::runtime_error( except.what() ));
        }

        const auto& vec = stringToVector< int >( dataInfo[ "sizes" ] );
        _volInfo.voxels[ 0 ] = vec[ 0 ];
        _volInfo.voxels[ 1 ] = vec[ 1 ];
        _volInfo.voxels[ 2 ] = vec[ 2 ];
        _volInfo.bigEndian = dataInfo[ "endian" ] == "big";
    }

    VolumeInformation& _volInfo;
    void* _mmapPtr;
    int32_t _fd;
    size_t _rawDataSize;
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
    return initData.getURI().getScheme() == "raw";
}

}
