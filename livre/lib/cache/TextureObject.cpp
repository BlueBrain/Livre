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
    : _lodTextureData( TextureDataObject::getEmptyPtr( ))
{
}

TextureObject::TextureObject( const CacheId& cacheId,
                              TextureCachePtr textureCache )
   : CacheObject( cacheId )
   , _textureCache( textureCache )
   , _textureState( new TextureState( ))
   , _lodTextureData( TextureDataObject::getEmptyPtr( ))
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

bool TextureObject::_isLoaded( ) const
{
    return _textureState->textureId != INVALID_TEXTURE_ID;
}

bool TextureObject::operator==( const TextureObject& texture ) const
{
    return _textureState == texture._textureState;
}

TextureStatePtr TextureObject::getTextureState( )
{
    return _textureState;
}

ConstTextureStatePtr TextureObject::getTextureState( ) const
{
    return _textureState;
}

void TextureObject::setTextureDataObject( ConstTextureDataObjectPtr lodTextureData )
{
    _lodTextureData = lodTextureData;
    _dataSource = _lodTextureData->getDataSource();

    const NodeId nodeId( getId( ));
    if( nodeId.getLevel() ==  0 )
        setUnloadable( false );
}

bool TextureObject::_load( )
{
    if( !_lodTextureData )
        return false;

    if( !_lodTextureData->isLoaded()
        || !_lodTextureData->isValid())
    {
        LBERROR << "Load texture is executed for invalid or unloaded data" << std::endl;
        return false;
    }

    if( !_textureState->texturePoolPtr.get() )
    {
        const Vector3i& maxBlockSize = _dataSource->getVolumeInformation().maximumBlockSize;
        const GLenum gpuDataType = _lodTextureData->getGPUDataType();
        GLenum format;
        switch( _dataSource->getVolumeInformation().compCount )
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

        TexturePoolPtr pool = _textureCache->getTexturePool( maxBlockSize,
                                                                format,
                                                                gpuDataType );
        _textureState->texturePoolPtr = pool;
    }

    _initialize( );

    _textureState->texturePoolPtr->generateTexture( _textureState );
    LBASSERT( _textureState->textureId );
    _loadTextureToGPU( );
    _lodTextureData.reset( TextureDataObject::getEmptyPtr() );
    return true;
}

void TextureObject::_unload( )
{
    LBASSERT( _isValid() );

    if( !_textureState->texturePoolPtr )
        return;

    _textureState->texturePoolPtr->releaseTexture( _textureState );
#ifdef _DEBUG_
    LBVERB << "Texture released : " << NodeId( getId( ))
           << " Last used at : " << getLastUsed()
           << " Number of references : " << getReferenceCount_( )
           << std::endl;

#endif //_DEBUG
}

void TextureObject::_initialize( )
{
    // TODO: The internal format size should be calculated correctly
    const Vector3f& overlap = _dataSource->getVolumeInformation().overlap;

    const ConstLODNodePtr& lodNode =
            _dataSource->getNode( NodeId( getId( )));

    const Vector3f& size = lodNode->getVoxelBox().getDimension();
    const Vector3f& maxSize = _textureState->texturePoolPtr->getMaxBlockSize();
    const Vector3f& overlapf = overlap / maxSize;
    _textureState->textureCoordsMax = size / maxSize;
    _textureState->textureCoordsMin = overlapf;
    _textureState->textureCoordsMax= _textureState->textureCoordsMax - overlapf;
    _textureState->textureSize = _textureState->textureCoordsMax - _textureState->textureCoordsMin;
}

GLenum TextureObject::getTextureType() const
{
    return _textureState->texturePoolPtr->getGPUDataType();
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
    switch( _textureState->texturePoolPtr->getGPUDataType() )
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

    const Vector3ui& textureSize =
            _textureState->texturePoolPtr->getMaxBlockSize();
    return textureSize.product() * elementSize;
}

const TextureDataObject& TextureObject::_getTextureDataObject() const
{
    return *static_cast< const TextureDataObject* >( _lodTextureData.get( ));
}

bool TextureObject::_loadTextureToGPU() const
{
    const ConstLODNodePtr& lodNode =
            _dataSource->getNode( NodeId( getId( )));

#ifdef LIVRE_DEBUG_RENDERING
    std::cout << "Upload "  << NodeId( getId( )).getLevel() << ' '
              << lodNode->getRelativePosition() << " to "
              << _textureState->textureId << std::endl;
#endif

    const Vector3i& voxSizeVec = lodNode->getVoxelBox().getDimension();

    _textureState->bind( );
    glTexSubImage3D( GL_TEXTURE_3D, 0, 0, 0, 0,
                     voxSizeVec[0], voxSizeVec[1], voxSizeVec[2],
                     _textureState->texturePoolPtr->getFormat() ,
                     _textureState->texturePoolPtr->getGPUDataType(),
                     _getTextureDataObject().getDataPtr( ));

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
