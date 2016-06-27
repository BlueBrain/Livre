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
#include <livre/lib/cache/TextureDataCache.h>

#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/maths/Quantizer.h>

#include <eq/gl.h>

namespace livre
{

struct TextureDataObject::Impl
{
public:

    Impl( TextureDataObject& dataObject,
          TextureDataCache& dataCache )
        : _data( new AllocMemoryUnit( ))
        , _dataObject( dataObject )
        , _dataCache( dataCache )
        , _dataSource( dataCache.getDataSource( ))
        , _gpuDataType( dataCache.getTextureType( ))
    {}

    bool isLoaded() const
    {
        return _data->getMemSize() > 0;
    }

    size_t getDataSize() const
    {
        const LODNode& lodNode =
                _dataSource.getNode( NodeId( _dataObject.getId( )));

        const Vector3ui& overlap =
                    _dataSource.getVolumeInfo().overlap;
        const size_t elemSize =
                    _dataSource.getVolumeInfo().getBytesPerVoxel();
        const uint32_t compCount =
                    _dataSource.getVolumeInfo().compCount;
        const Vector3ui blockSize =
                    lodNode.getBlockSize() + overlap * 2;
        return blockSize.product() * elemSize * compCount;
    }

    size_t getSize() const
    {
        return getDataSize();
    }

    const void* getDataPtr() const
    {
        return _data->getData< void >();
    }

    template< class DEST_TYPE >
    bool readTextureData( const bool quantize )
    {
        const NodeId nodeId( _dataObject.getId( ));
        ConstMemoryUnitPtr data = _dataSource.getData( nodeId );
        if( !data )
            return false;

        const void* rawData = data->getData< void >();
        if( quantize )
        {
            std::vector< DEST_TYPE > textureData;
            convertData( rawData, textureData );
            _data->allocAndSetData( textureData );
        }
        else
            _data->allocAndSetData( rawData, getDataSize( ));

        return true;
    }

    template< class SRC_TYPE, class DEST_TYPE >
    void quantizeData( const SRC_TYPE* rawData,
                       std::vector< DEST_TYPE >& formattedData ) const
    {
        const VolumeInformation& volumeInfo = _dataSource.getVolumeInfo();
        const uint32_t compCount = volumeInfo.compCount;
        const size_t dataSize = getDataSize();

        const Vector3f min( std::numeric_limits< DEST_TYPE >::min( ));
        const Vector3f max( std::numeric_limits< DEST_TYPE >::max( ));
        if( std::is_signed< DEST_TYPE >::value )
        {
            signedQuantize( rawData, &formattedData[ 0 ], dataSize,
                            compCount, min, max );
        }
        else
        {
            unsignedQuantize( rawData, &formattedData[ 0 ], dataSize,
                              compCount, min, max );
        }
    }

    template< class DEST_TYPE >
    void convertData( const void* rawData,
                      std::vector< DEST_TYPE >& formattedData ) const
    {

        const VolumeInformation& volumeInfo = _dataSource.getVolumeInfo();
        const DataType dataType = volumeInfo.dataType;
        const size_t dataSize = getDataSize();
        formattedData.resize( dataSize );

        switch( dataType )
        {
           case DT_UINT8:
                quantizeData< uint8_t, DEST_TYPE >( static_cast< const uint8_t* >( rawData ),
                                                    formattedData );
                break;
           case DT_UINT16:
                quantizeData< uint16_t, DEST_TYPE >( static_cast< const uint16_t* >( rawData ),
                                                     formattedData );
                break;
           case DT_UINT32:
                quantizeData< uint32_t, DEST_TYPE >( static_cast< const uint32_t* >( rawData ),
                                                     formattedData );
                break;
           case DT_INT8:
                quantizeData< int8_t, DEST_TYPE >( static_cast< const int8_t* >( rawData ),
                                                   formattedData );
                break;
           case DT_INT16:
                quantizeData< int16_t, DEST_TYPE >( static_cast< const int16_t* >( rawData ),
                                                    formattedData );
                break;
           case DT_INT32:
                quantizeData< int32_t, DEST_TYPE >( static_cast< const int32_t* >( rawData ),
                                                    formattedData );
                break;
           case DT_UNDEFINED:
           case DT_FLOAT32:
           case DT_FLOAT64:
           {
                LBTHROW( std::runtime_error( "Unimplemented data type." ));
           }
        }
    }

    bool load()
    {
        const DataType dataType = _dataSource.getVolumeInfo().dataType;
        switch( _gpuDataType )
        {
            case GL_UNSIGNED_BYTE:
                return readTextureData< uint8_t >( dataType != DT_UINT8 );
            case GL_FLOAT:
                return readTextureData< float >( dataType != DT_FLOAT32 );
            case GL_UNSIGNED_SHORT:
                return readTextureData< uint16_t >( dataType != DT_UINT16 );
        }
        return false;
    }

    void unload()
    {
        _data->release();

        const NodeId nodeId( _dataObject.getId( ));
           LBVERB << "Texture Data released: " << nodeId
               << std::endl;
    }

    AllocMemoryUnitPtr _data;
    TextureDataObject& _dataObject;
    TextureDataCache& _dataCache;
    DataSource& _dataSource;
    uint32_t _gpuDataType;
};

TextureDataObject::TextureDataObject( const CacheId& cacheId,
                                     TextureDataCache& dataCache )
    : CacheObject( cacheId )
    , _impl( new Impl( *this, dataCache ))
{}

TextureDataObject::~TextureDataObject()
{}

bool TextureDataObject::_isLoaded( ) const
{
    if( !_isValid( ))
        return 0;

    return _impl->isLoaded();
}

size_t TextureDataObject::_getSize() const
{
    if( !_isValid( ))
        return 0;

    return _impl->getSize();
}

const void* TextureDataObject::getDataPtr() const
{
    return _impl->getDataPtr();
}

bool TextureDataObject::_load()
{
    return _impl->load();
}

void TextureDataObject::_unload()
{
    return _impl->unload();
}

}
