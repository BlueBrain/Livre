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

#include <livre/lib/cache/TextureCache.h>
#include <livre/lib/cache/TextureDataCache.h>
#include <livre/lib/cache/TextureDataObject.h>
#include <livre/lib/cache/TextureObject.h>

#include <livre/core/data/LODNode.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/render/GLContext.h>
#include <livre/core/render/Renderer.h>
#include <livre/core/render/TexturePool.h>

#include <eq/gl.h>

namespace livre
{

#define glewGetContext() GLContext::glewGetContext()

/**
 * The TextureObject class holds the informarmation for the data which is on the GPU.
  */
struct TextureObject::Impl
{
    Impl( TextureObject& textureObject,
          TextureCache& textureCache )
       : _textureObject( textureObject )
       , _textureCache( textureCache )
       , _textureState( new TextureState( ))
       , _dataSource( textureCache.getDataCache().getDataSource( ))
       , _texturePool( textureCache.getTexturePool( ))
       , _textureType( textureCache.getDataCache( ).getTextureType( ))
    {}

    bool isLoaded() const
    {
        return _textureState && _textureState->textureId != INVALID_TEXTURE_ID;
    }

    bool load()
    {
        _dataObject = std::static_pointer_cast< const TextureDataObject >(
                    _textureCache.getDataCache().get( _textureObject.getId( )));

        if( !_dataObject->isLoaded( ))
            return false;

        initialize( );
        _texturePool.generateTexture( _textureState );
        LBASSERT( _textureState->textureId );
        loadTextureToGPU();
        _dataObject.reset();
        return true;
    }

    void unload()
    {
        if( _textureState && _textureState->textureId == INVALID_TEXTURE_ID )
            return;

        _texturePool.releaseTexture( _textureState );
    }

    void initialize()
    {
        // TODO: The internal format size should be calculated correctly
        const Vector3f& overlap = _dataSource.getVolumeInfo().overlap;

        const LODNode& lodNode =
                _dataSource.getNode( NodeId( _textureObject.getId( )));

        const Vector3f& size = lodNode.getVoxelBox().getSize();
        const Vector3f& maxSize = _dataSource.getVolumeInfo().maximumBlockSize;
        const Vector3f& overlapf = overlap / maxSize;
        _textureState->textureCoordsMax = overlapf + size / maxSize;
        _textureState->textureCoordsMin = overlapf;
        _textureState->textureSize =
                _textureState->textureCoordsMax - _textureState->textureCoordsMin;
    }

    size_t getSize() const
    {
        // L (luminance, e.g. greyscale): width * height * type size.
        // LA (luminance and alpha, common for fonts): width * height * type size bytes.
        // RGB (color, no alpha): width * height * type size bytes.
        // RGBA (color with alpha): width * height * type size bytes.

        uint32_t elementSize = 0;
        switch( _textureType )
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
                _dataSource.getVolumeInfo().maximumBlockSize;
        return textureSize.product() * elementSize;
    }

    bool loadTextureToGPU() const
    {
        const NodeId nodeId( _textureObject.getId( ));
        const LODNode& lodNode = _dataSource.getNode( nodeId );

    #ifdef LIVRE_DEBUG_RENDERING
        std::cout << "Upload "  << nodeId.getLevel() << ' '
                  << lodNode.getRelativePosition() << " to "
                  << _textureState->textureId << std::endl;
    #endif
        const Vector3ui& overlap = _dataSource.getVolumeInfo().overlap;

        const Vector3ui& voxSizeVec =
                lodNode.getBlockSize() + overlap * 2;

        _textureState->bind( );
        glTexSubImage3D( GL_TEXTURE_3D, 0, 0, 0, 0,
                         voxSizeVec[0], voxSizeVec[1], voxSizeVec[2],
                         _texturePool.getFormat() ,
                         _texturePool.getGPUDataType(),
                         _dataObject->getDataPtr( ));

        // Something went wrong with loading the data
        // TODO: Log message
        const GLenum glErr = glGetError();
        if ( glErr != GL_NO_ERROR )
        {
            LBERROR << "Error loading the texture into GPU, error number : "  << glErr << std::endl;
            return false;
        }

        return true;
    }

    TextureObject& _textureObject;
    TextureCache& _textureCache;
    ConstTextureDataObjectPtr _dataObject;
    TextureStatePtr _textureState;
    DataSource& _dataSource;
    TexturePool& _texturePool;
    uint32_t _textureType;
};


TextureObject::TextureObject( const CacheId& cacheId,
                              TextureCache& textureCache )
   : CacheObject( cacheId )
   , _impl( new Impl( *this, textureCache ))
{
}

TextureObject::~TextureObject()
{
}

bool TextureObject::_isLoaded() const
{
    return _impl->isLoaded();
}

TextureStatePtr TextureObject::getTextureState()
{
    return _impl->_textureState;
}

ConstTextureStatePtr TextureObject::getTextureState() const
{
    return _impl->_textureState;
}

bool TextureObject::_load()
{
    return _impl->load();
}

void TextureObject::_unload()
{
    _impl->unload();
}

size_t TextureObject::_getSize() const
{
    if( !_isValid() )
        return 0;

    return _impl->getSize();
}

}
