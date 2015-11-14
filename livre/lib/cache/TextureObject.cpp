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

#include <livre/lib/cache/TextureCache.h>
#include <livre/lib/cache/TextureDataObject.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/data/LODNode.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/data/VolumeDataSource.h>
#include <livre/core/render/GLContext.h>
#include <livre/core/render/Renderer.h>
#include <livre/core/render/TexturePool.h>

#include <eq/gl.h>

namespace livre
{

#define glewGetContext() GLContext::glewGetContext()

TextureObject::TextureObject()
    : lodTextureData_( TextureDataObject::getEmptyPtr( ))
{
}

TextureObject::TextureObject( const CacheId& cacheId,
                              TextureCachePtr textureCachePtr )
   : CacheObject( cacheId )
   , textureCachePtr_( textureCachePtr )
   , textureState_( new TextureState( ))
   , lodTextureData_( TextureDataObject::getEmptyPtr( ))
{
}

TextureObject::~TextureObject()
{
}

TextureObject* TextureObject::getEmptyPtr()
{
    static boost::shared_ptr< TextureObject > texture( new TextureObject );
    return texture.get();
}

bool TextureObject::isLoaded_( ) const
{
    return textureState_->textureId != INVALID_TEXTURE_ID;
}

bool TextureObject::isValid_( ) const
{
    return textureState_.get();
}

bool TextureObject::operator==( const TextureObject& texture ) const
{
    return textureState_ == texture.textureState_;
}

TextureStatePtr TextureObject::getTextureState( )
{
    return textureState_;
}

ConstTextureStatePtr TextureObject::getTextureState( ) const
{
    return textureState_;
}

void TextureObject::setTextureDataObject( ConstTextureDataObjectPtr lodTextureData )
{
    lodTextureData_ = lodTextureData;
    dataSourcePtr_ = lodTextureData_->getDataSource();

    if( NodeId( getCacheId( )).getLevel() ==  0 )
        setUnloadable( false );
}

bool TextureObject::load_( )
{
    if( !lodTextureData_ )
        return false;

    if( !lodTextureData_->isLoaded()
        || !lodTextureData_->isValid())
    {
        LBERROR << "Load texture is executed for invalid or unloaded data" << std::endl;
        return false;
    }

    if( !textureState_->texturePoolPtr.get() )
    {
        const Vector3i& maxBlockSize = dataSourcePtr_->getVolumeInformation().maximumBlockSize;
        const GLenum gpuDataType = lodTextureData_->getGPUDataType();
        GLenum format;
        switch( dataSourcePtr_->getVolumeInformation().compCount )
        {
            case 1:
                format = GL_RED;
                break;
            case 3:
                format = GL_RGB;
                break;
            default:
                format = GL_RED;
                break;
        }

        TexturePoolPtr pool = textureCachePtr_->getTexturePool( maxBlockSize,
                                                                format,
                                                                gpuDataType );
        textureState_->texturePoolPtr = pool;
    }

    initialize_( );

    textureState_->texturePoolPtr->generateTexture( textureState_ );
    LBASSERT( textureState_->textureId );
    loadTextureToGPU_( );
    lodTextureData_.reset( TextureDataObject::getEmptyPtr() );
    return true;
}

void TextureObject::unload_( )
{
    LBASSERT( isValid_() );

    if( !textureState_->texturePoolPtr )
        return;

    textureState_->texturePoolPtr->releaseTexture( textureState_ );
}

void TextureObject::initialize_( )
{
    // TODO: The internal format size should be calculated correctly
    const Vector3f& overlap = dataSourcePtr_->getVolumeInformation().overlap;
    const LODNode& lodNode = dataSourcePtr_->getNode( NodeId( getCacheId( )));
    const Vector3f& size = lodNode.getVoxelBox( ).getDimension( );
    const Vector3f& maxSize = textureState_->texturePoolPtr->getMaxBlockSize();
    const Vector3f& overlapf = overlap / maxSize;
    textureState_->textureCoordsMax = size / maxSize;
    textureState_->textureCoordsMin = overlapf;
    textureState_->textureCoordsMax= textureState_->textureCoordsMax - overlapf;
    textureState_->textureSize = textureState_->textureCoordsMax - textureState_->textureCoordsMin;
}

GLenum TextureObject::getTextureType() const
{
    return textureState_->texturePoolPtr->getGPUDataType();
}

size_t TextureObject::getCacheSize( ) const
{
    if( !isValid() )
        return 0;

    // L (luminance, e.g. greyscale): width * height * type size.
    // LA (luminance and alpha, common for fonts): width * height * type size bytes.
    // RGB (color, no alpha): width * height * type size bytes.
    // RGBA (color with alpha): width * height * type size bytes.

    uint32_t elementSize = 0;
    switch( textureState_->texturePoolPtr->getGPUDataType() )
    {
        case GL_UNSIGNED_BYTE:
            elementSize = sizeof( char );
            break;
        case GL_FLOAT:
            elementSize = sizeof( float );
            break;
        case GL_UNSIGNED_SHORT:
            elementSize = sizeof( short );
            break;
    }

    const Vector3i textureSize = textureState_->texturePoolPtr->getMaxBlockSize();
    const uint32_t cacheSize = textureSize[ 0 ] * textureSize[ 1 ] * textureSize[ 2 ] * elementSize;
    return cacheSize;
}

const TextureDataObject& TextureObject::getTextureDataObject_( ) const
{
    return *static_cast< const TextureDataObject* >( lodTextureData_.get() );
}


bool TextureObject::loadTextureToGPU_( ) const
{
    const LODNode& lodNode = dataSourcePtr_->getNode( NodeId( getCacheId( )));

#ifdef LIVRE_DEBUG_RENDERING
    std::cout << "Upload "  << lodNode.getRefLevel() << ' '
              << lodNode.getRelativePosition() << " to "
              << textureState_->textureId << std::endl;
#endif

    const Vector3i& voxSizeVec = lodNode.getVoxelBox( ).getDimension( );

    textureState_->bind( );
    glTexSubImage3D( GL_TEXTURE_3D, 0, 0, 0, 0,
                     voxSizeVec[0], voxSizeVec[1], voxSizeVec[2],
                     textureState_->texturePoolPtr->getFormat() ,
                     textureState_->texturePoolPtr->getGPUDataType(),
                     getTextureDataObject_().getDataPtr( ));

    // Something went wrong with loading the data
    // TODO: Log message
    const GLenum glErr = glGetError();
    if ( glErr != GL_NO_ERROR )
    {
        LBERROR << "Error loading the texture into GPU, error number : "  << glErr << std::endl;
        return false;
    }

    glFinish( );
    return true;
}

}
