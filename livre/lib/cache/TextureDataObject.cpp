/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include <livre/lib/cache/TextureDataObject.h>

#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/maths/Quantizer.h>

#include <eq/gl.h>

namespace livre
{
TextureDataObject::TextureDataObject()
    : CacheObject()
    , _data( new AllocMemoryUnit( ))
    , _dataSource()
    , _gpuDataType( 0 )
{
}

TextureDataObject::TextureDataObject( const CacheId& cacheId,
                                      VolumeDataSourcePtr dataSourcePtr,
                                      const uint32_t gpuDataType )
    : CacheObject( cacheId )
    , _data( new AllocMemoryUnit( ))
    , _dataSource( dataSourcePtr )
    , _gpuDataType( gpuDataType )
{
    if( NodeId( getId( )).getLevel() ==  0 )
        setUnloadable( false );
}

TextureDataObject::~TextureDataObject()
{
}

TextureDataObject* TextureDataObject::getEmptyPtr()
{
    static boost::shared_ptr< TextureDataObject > data( new TextureDataObject( ));
    return data.get();
}

bool TextureDataObject::operator==( const TextureDataObject& data ) const
{
    return getId() == data.getId();
}

bool TextureDataObject::_isLoaded( ) const
{
    return _data->getMemSize();
}

size_t TextureDataObject::_getDataSize() const
{
    if( !isValid() )
        return 0;

    const LODNode& lodNode =
            _dataSource->getNode( NodeId( getId( )));

    const Vector3ui& overlap =
                _dataSource->getVolumeInformation().overlap;
    const uint32_t elemSize =
                _dataSource->getVolumeInformation().getBytesPerVoxel();
    const Vector3ui blockSize =
                lodNode.getBlockSize() + overlap * 2;
    return blockSize.product() * elemSize;
}

size_t TextureDataObject::getSize() const
{
    if( !isValid() )
        return 0;

    return _data->getAllocSize();
}

ConstVolumeDataSourcePtr TextureDataObject::getDataSource() const
{
    return _dataSource;
}

GLenum TextureDataObject::getGPUDataType() const
{
    return _gpuDataType;
}

const void* TextureDataObject::getDataPtr() const
{
    _getUnconst()->touch();
    return _data->getData< void >();
}

template< class T >
bool TextureDataObject::_setTextureData( const bool quantize )
{
    _getUnconst()->touch();
    const NodeId nodeId( getId( ));
    ConstMemoryUnitPtr data = _dataSource->getData( nodeId );
    if( !data )
        return false;

    const T* rawData = data->getData< T >();
    if( quantize )
    {
        std::vector< T > textureData;
        _getQuantizedData< T >( rawData, textureData );
        _data->allocAndSetData( textureData );
    }
    else
        _data->allocAndSetData( rawData, _getRawDataSize( ));
    return true;
}

template< class T >
void TextureDataObject::_getQuantizedData( const T* rawData,
                                         std::vector< T >& formattedData ) const
{
    const VolumeInformation& volumeInfo = _dataSource->getVolumeInformation();
    const uint32_t compCount = volumeInfo.compCount;
    const DataType dataType = volumeInfo.dataType;
    const size_t dataSize = _getRawDataSize();

    formattedData.resize( dataSize );

    switch( dataType )
    {
       case DT_UINT8:
       {
            const Vector3f min( std::numeric_limits< uint8_t >::min( ));
            const Vector3f max( std::numeric_limits< uint8_t >::max( ));
            unsignedQuantize( rawData, &formattedData[ 0 ], dataSize,
                              compCount, min, max );
            break;
       }
       case DT_UINT16:
       {
            const Vector3f min( std::numeric_limits< uint16_t >::min( ));
            const Vector3f max( std::numeric_limits< uint16_t >::max( ));
            unsignedQuantize( rawData, &formattedData[ 0 ], dataSize,
                              compCount, min, max );
            break;
       }
       case DT_UINT32:
       {
            const Vector3f min( std::numeric_limits< uint32_t >::min( ));
            const Vector3f max( std::numeric_limits< uint32_t >::max( ));
            unsignedQuantize( rawData, &formattedData[ 0 ], dataSize,
                              compCount, min, max );
            break;
       }
       case DT_INT8:
       {
            const Vector3f min( std::numeric_limits< int8_t >::min( ));
            const Vector3f max( std::numeric_limits< int8_t >::max( ));
            signedQuantize( rawData, &formattedData[ 0 ], dataSize,
                            compCount, min, max );
            break;
       }
       case DT_INT16:
       {
            const Vector3f min( std::numeric_limits< int16_t >::min( ));
            const Vector3f max( std::numeric_limits< int16_t >::max( ));
            signedQuantize( rawData, &formattedData[ 0 ], dataSize,
                            compCount, min, max);
            break;
       }
       case DT_INT32:
       {
            const Vector3f min( std::numeric_limits< int32_t >::min( ));
            const Vector3f max( std::numeric_limits< int32_t >::max( ));
            signedQuantize( rawData, &formattedData[ 0 ], dataSize,
                            compCount, min, max );
            break;
       }
       case DT_UNDEFINED:
       case DT_FLOAT32:
       case DT_FLOAT64:
       {
            LBTHROW( std::runtime_error( "Unimplemented data type." ));
       }
    }
}

size_t TextureDataObject::_getRawDataSize() const
{
    const VolumeInformation& volumeInfo = _dataSource->getVolumeInformation();
    return _getDataSize() * volumeInfo.compCount * volumeInfo.getBytesPerVoxel();
}

bool TextureDataObject::_load( )
{
    const DataType dataType = _dataSource->getVolumeInformation().dataType;
    switch( _gpuDataType )
    {
        case GL_UNSIGNED_BYTE:
            return _setTextureData< uint8_t >( dataType != DT_UINT8 );
        case GL_FLOAT:
            return _setTextureData< float >( dataType != DT_FLOAT32 );
        case GL_UNSIGNED_SHORT:
            return _setTextureData< uint16_t >( dataType != DT_UINT16 );
    }
    return false;
}

void TextureDataObject::_unload( )
{
    _data->release();

    const NodeId nodeId( getId( ));
       LBVERB << "Texture Data released: " << nodeId
           << std::endl;
}

}
