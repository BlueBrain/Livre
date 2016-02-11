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
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/maths/Quantizer.h>

#include <eq/gl.h>

namespace livre
{

struct TextureDataObject::Impl
{
public:

    Impl( TextureDataObject& dataObject,
          TextureDataCache& dataCache )
        : _data( new AllocMemoryUnit())
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
                    _dataSource.getVolumeInformation().overlap;
        const size_t elemSize =
                    _dataSource.getVolumeInformation().getBytesPerVoxel();
        const uint32_t compCount =
                    _dataSource.getVolumeInformation().compCount;
        const Vector3ui blockSize =
                    lodNode.getBlockSize() + overlap * 2;
        return blockSize.product() * elemSize * compCount;
    }

    size_t getSize() const
    {
        return _data->getAllocSize();
    }

    const void* getDataPtr() const
    {
        return _data->getData< void >();
    }

    template< class T >
    bool setTextureData( const bool quantize )
    {
        const NodeId nodeId( _dataObject.getId( ));
        ConstMemoryUnitPtr data = _dataSource.getData( nodeId );
        if( !data )
            return false;

        const T* rawData = data->getData< T >();
        if( quantize )
        {
            std::vector< T > textureData;
            getQuantizedData< T >( rawData, textureData );
            _data->allocAndSetData( textureData );
        }
        else
            _data->allocAndSetData( rawData, getDataSize( ));
        return true;
    }

    template< class T >
    void getQuantizedData( const T* rawData,
                           std::vector< T >& formattedData ) const
    {
        const VolumeInformation& volumeInfo = _dataSource.getVolumeInformation();
        const uint32_t compCount = volumeInfo.compCount;
        const DataType dataType = volumeInfo.dataType;
        const size_t dataSize = getDataSize();

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

    bool load()
    {
        const DataType dataType = _dataSource.getVolumeInformation().dataType;
        switch( _gpuDataType )
        {
            case GL_UNSIGNED_BYTE:
                return setTextureData< uint8_t >( dataType != DT_UINT8 );
            case GL_FLOAT:
                return setTextureData< float >( dataType != DT_FLOAT32 );
            case GL_UNSIGNED_SHORT:
                return setTextureData< uint16_t >( dataType != DT_UINT16 );
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
    VolumeDataSource& _dataSource;
    uint32_t _gpuDataType;
};

TextureDataObject::TextureDataObject( const CacheId& cacheId,
                                     TextureDataCache& dataCache  )
    : CacheObject( cacheId )
    , _impl( new Impl( *this, dataCache ))
{
    if( NodeId( getId( )).getLevel() ==  0 )
        setUnloadable( false );
}

TextureDataObject::~TextureDataObject()
{
}

bool TextureDataObject::_isLoaded( ) const
{
    if( !isValid( ))
        return 0;

    return _impl->isLoaded();
}

size_t TextureDataObject::getSize() const
{
    if( !isValid( ))
        return 0;

    return _impl->getSize();
}

const void* TextureDataObject::getDataPtr() const
{
    _getUnconst()->touch();
    return _impl->getDataPtr();
}

bool TextureDataObject::_load()
{
    _getUnconst()->touch();
    return _impl->load();
}

void TextureDataObject::_unload()
{
    return _impl->unload();
}

}
