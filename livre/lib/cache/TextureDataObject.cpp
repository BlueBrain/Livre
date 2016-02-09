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
    , data_( new AllocMemoryUnit( ))
    , dataSourcePtr_()
    , gpuDataType_( 0 )
{
}

TextureDataObject::TextureDataObject( const CacheId& cacheId,
                                      VolumeDataSourcePtr dataSourcePtr,
                                      const uint32_t gpuDataType )
    : CacheObject( cacheId )
    , data_( new AllocMemoryUnit( ))
    , dataSourcePtr_( dataSourcePtr )
    , gpuDataType_( gpuDataType )
{
    if( NodeId( getCacheId( )).getLevel() ==  0 )
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
    return getCacheId() == data.getCacheId();
}

bool TextureDataObject::isLoaded_( ) const
{
    return data_->getMemSize();
}

size_t TextureDataObject::getDataSize_() const
{
    if( !isValid() )
        return 0;

    const ConstLODNodePtr& lodNode =
            dataSourcePtr_->getNode( NodeId( getCacheId( )));

    const Vector3ui& overlap =
                dataSourcePtr_->getVolumeInformation().overlap;
    const size_t elemSize =
                dataSourcePtr_->getVolumeInformation().getBytesPerVoxel();
    const Vector3ui blockSize =
                lodNode->getBlockSize() + overlap * 2;
    return blockSize.product() * elemSize;
}

size_t TextureDataObject::getCacheSize() const
{
    if( !isValid() )
        return 0;

    return data_->getAllocSize();
}

ConstVolumeDataSourcePtr TextureDataObject::getDataSource() const
{
    return dataSourcePtr_;
}

GLenum TextureDataObject::getGPUDataType() const
{
    return gpuDataType_;
}

const void* TextureDataObject::getDataPtr() const
{
    getUnconst_()->updateLastUsedWithCurrentTime_();
    return data_->getData< void >();
}

template< class T >
bool TextureDataObject::setTextureData_( const bool quantize )
{
    getUnconst_()->updateLastUsedWithCurrentTime_();
    const ConstLODNodePtr& lodNode =
            dataSourcePtr_->getNode( NodeId( getCacheId( )));

    ConstMemoryUnitPtr data = dataSourcePtr_->getData( *lodNode );
    if( !data )
        return false;

    const T* rawData = data->getData< T >();
    if( quantize )
    {
        std::vector< T > textureData;
        getQuantizedData_< T >( rawData, textureData );
        data_->allocAndSetData( textureData );
    }
    else
        data_->allocAndSetData( rawData, getRawDataSize_( ));
    return true;
}

template< class T >
void TextureDataObject::getQuantizedData_( const T* rawData,
                                         std::vector< T >& formattedData ) const
{
    const VolumeInformation& volumeInfo = dataSourcePtr_->getVolumeInformation();
    const uint32_t compCount = volumeInfo.compCount;
    const DataType dataType = volumeInfo.dataType;
    const size_t dataSize = getRawDataSize_();

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

size_t TextureDataObject::getRawDataSize_() const
{
    const VolumeInformation& volumeInfo = dataSourcePtr_->getVolumeInformation();
    return getDataSize_() * volumeInfo.compCount * volumeInfo.getBytesPerVoxel();
}

bool TextureDataObject::load_( )
{
    const DataType dataType = dataSourcePtr_->getVolumeInformation().dataType;
    switch( gpuDataType_ )
    {
        case GL_UNSIGNED_BYTE:
            return setTextureData_< uint8_t >( dataType != DT_UINT8 );
        case GL_FLOAT:
            return setTextureData_< float >( dataType != DT_FLOAT32 );
        case GL_UNSIGNED_SHORT:
            return setTextureData_< uint16_t >( dataType != DT_UINT16 );
    }
    return false;
}

void TextureDataObject::unload_( )
{
    data_->release();

    const ConstLODNodePtr& lodNode =
            dataSourcePtr_->getNode( NodeId( getCacheId( )));

    LBVERB << "Texture Data released: " << lodNode->getNodeId()
           << std::endl;
}

}
