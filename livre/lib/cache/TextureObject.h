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

#ifndef _TextureObject_h_
#define _TextureObject_h_

#include <livre/lib/api.h>
#include <livre/lib/types.h>
#include <livre/core/mathTypes.h>
#include <livre/core/data/LODNodeTrait.h>
#include <livre/core/cache/CacheObject.h>

namespace livre
{

/**
 * The TextureObject class holds the informarmation for the data which is on the GPU.
  */
class TextureObject : public CacheObject
{
    friend class TextureCache;

public:
    LIVRE_API TextureObject();

    LIVRE_API virtual ~TextureObject();

    /**
     * @return True if two data object has the same cache id.
     */
    LIVRE_API bool operator==( const TextureObject& texture ) const;

    /**
     * @return The GPU memory usage.
     */
    size_t getSize() const final;

    /**
     * @return The texture type.
     */
    LIVRE_API uint32_t getTextureType() const;

    /**
     * @return The texture state.
     */
    LIVRE_API TextureStatePtr getTextureState();

    /**
     * @return The texture state ( const ).
     */
    LIVRE_API ConstTextureStatePtr getTextureState() const;

    /**
     * Sets the texture data object for data retrieval.
     * @param lodTextureData livre::TextureDataObject.
     */
    LIVRE_API void setTextureDataObject( ConstTextureDataObjectPtr lodTextureData );

    /**
     * @return An empty data object ptr.
     */
    static TextureObject* getEmptyPtr();

private:

    TextureObject( const CacheId& cacheId,
                   TextureCachePtr textureCache );
    const TextureDataObject& _getTextureDataObject() const;

    bool _load( ) final;
    void _unload( ) final;
    bool _isLoaded( ) const final;
    bool _loadTextureToGPU( ) const;
    void _initialize( );

    TextureCachePtr _textureCache;
    TextureStatePtr _textureState;
    ConstTextureDataObjectPtr _lodTextureData;
    ConstVolumeDataSourcePtr _dataSource;
};

}

#endif // _TextureObject_h_
