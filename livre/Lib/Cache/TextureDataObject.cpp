/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/Lib/Cache/TextureDataObject.h>
#include <livre/Lib/Cache/TextureObject.h>
#include <livre/Lib/Cache/RawDataObject.h>

#include <livre/core/Data/LODNode.h>
#include <livre/core/Data/VolumeDataSource.h>

#include <eq/gl.h>

namespace livre
{
TextureDataObject::TextureDataObject( )
    : rawDataObject_( RawDataObject::getEmptyPtr() )
{
    data_.reset( new AllocMemoryUnit( ) );
}

TextureDataObject::TextureDataObject( GLenum gpuDataType )
    : rawDataObject_( RawDataObject::getEmptyPtr() ),
      gpuDataType_( gpuDataType )
{
    data_.reset( new AllocMemoryUnit( ) );
}

TextureDataObject::~TextureDataObject()
{
}

TextureDataObject* TextureDataObject::getEmptyPtr()
{
    static boost::shared_ptr< TextureDataObject > data( new TextureDataObject() );
    return data.get();
}

CacheId TextureDataObject::getCacheID() const
{
    return lodNodePtr_->getNodeId().getId();
}

bool TextureDataObject::operator==( const TextureDataObject& data ) const
{
    return lodNodePtr_ == data.lodNodePtr_;
}

bool TextureDataObject::isLoaded_( ) const
{
    return data_->getMemSize();
}

bool TextureDataObject::isValid_( ) const
{
    return lodNodePtr_->isValid();
}

uint32_t TextureDataObject::getDataSize() const
{
    if( !isValid() )
        return 0;

    const Vector3i& voxSizeVec = lodNodePtr_->getVoxelBox( ).getDimension( );
    return voxSizeVec[0] * voxSizeVec[1] * voxSizeVec[2];
}

uint32_t TextureDataObject::getCacheSize() const
{
    if( !isValid() )
        return 0;

    return data_->getAllocSize();
}

ConstVolumeDataSourcePtr TextureDataObject::getDataSource() const
{
    return dataSourcePtr_;
}

void TextureDataObject::setRawData( ConstRawDataObjectPtr rawDataObject )
{
    rawDataObject_ = rawDataObject;

    if( !lodNodePtr_->isValid() )
        lodNodePtr_ = rawDataObject_->getLODNode();

    dataSourcePtr_ = rawDataObject_->getDataSource();

    if( lodNodePtr_->getRefLevel() ==  0 )
        setUnloadable( false );
}

GLenum TextureDataObject::getGPUDataType() const
{
    return gpuDataType_;
}

template< class T >
void TextureDataObject::setTextureData_( bool quantize, bool normalize )
{
    if( quantize || normalize )
    {
        std::vector< T > textureData;
        rawDataObject_->getQuantizedData< T >( textureData );
        data_->allocAndSetData( textureData );
    }
    else
    {
        const T* ptr = rawDataObject_->getDataPtr< T >( );
        data_->allocAndSetData( ptr, getDataSize( ) * sizeof( T ) );
    }
    rawDataObject_.reset( RawDataObject::getEmptyPtr() );
}

bool TextureDataObject::load_( )
{
    if( !rawDataObject_->isLoaded() )
        return false;

    const DataType dataType = dataSourcePtr_->getVolumeInformation().dataType;
    switch( gpuDataType_ )
    {
        case GL_UNSIGNED_BYTE:
            setTextureData_< uint8_t >( dataType != DT_UINT8, false );
            break;
        case GL_FLOAT:
            setTextureData_< float >( dataType != DT_FLOAT32, false );
            break;
        case GL_UNSIGNED_SHORT:
            setTextureData_< uint16_t >( dataType != DT_UINT16, false );
            break;
    }
    return true;
}

void TextureDataObject::unload_( )
{
    data_->release();
#ifdef _DEBUG_
    LBVERB << "Texture Data released: " << lodNodePtr_->getNodeId() << std::endl;
#endif
}

const RawDataObject& TextureDataObject::getRawDataObject_( ) const
{
    return *static_cast< const RawDataObject * >( rawDataObject_.get() );
}

}
