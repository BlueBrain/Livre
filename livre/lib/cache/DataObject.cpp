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

#include <livre/lib/cache/DataObject.h>
#include <livre/lib/cache/DataCache.h>

#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/maths/Quantizer.h>

#include <eq/gl.h>

namespace livre
{

struct DataObject::Impl
{
public:

    Impl( DataObject& dataObject,
          DataCache& dataCache )
        : _data( new AllocMemoryUnit( ))
        , _dataObject( dataObject )
        , _dataCache( dataCache )
        , _dataSource( dataCache.getDataSource( ))
        , _gpuDataType( dataCache.getTextureType( ))
    {
        if( !load())
        {
            LBTHROW( CacheLoadException( dataObject.getId(),
                                         "Unable to construct data cache object" ));
        }
    }

    ~Impl()
    {
        _data->release();
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

    AllocMemoryUnitPtr _data;
    DataObject& _dataObject;
    DataCache& _dataCache;
    DataSource& _dataSource;
    uint32_t _gpuDataType;
};

DataObject::DataObject( const CacheId& cacheId,
                        DataCache& dataCache )
    : CacheObject( cacheId )
    , _impl( new Impl( *this, dataCache ))
{}

DataObject::~DataObject()
{}

size_t DataObject::_getSize() const
{
    if( !_isValid( ))
        return 0;

    return _impl->getSize();
}

const void* DataObject::getDataPtr() const
{
    return _impl->getDataPtr();
}

}
