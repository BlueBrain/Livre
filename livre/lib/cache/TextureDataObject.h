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

#ifndef _TextureDataObject_h_
#define _TextureDataObject_h_

#include <livre/lib/types.h>

#include <livre/core/cache/CacheObject.h> // base class

namespace livre
{

/**
 * The TextureDataObject class gets raw data from the volume data source and
 * stores the quantized/formatted data for the GPU.
 */
class TextureDataObject : public CacheObject
{
public:
    ~TextureDataObject();

    /** @return True if two data object has the same cache id. */
    bool operator==( const TextureDataObject& data ) const;

    /** @return The data source. */
    ConstVolumeDataSourcePtr getDataSource() const;

    /** @return The GPU data type */
    uint32_t getGPUDataType() const;

    /** @return A pointer to the data or 0 if no data is loaded. */
    const void* getDataPtr() const;

    /** @return An empty data object ptr. */
    static TextureDataObject* getEmptyPtr();

private:
    friend class TextureDataCache;

    TextureDataObject();
    TextureDataObject( const CacheId& cacheId,
                       VolumeDataSourcePtr dataSourcePtr,
                       uint32_t gpuDataType );

    bool _load() final;
    void _unload() final;
    bool _isLoaded() const final;
    size_t getCacheSize() const final;

    template< class T >
    bool _setTextureData( bool quantize );

    size_t _getDataSize() const;
    size_t _getRawDataSize() const;

    /**
     * Quantizes data into the given format with T.
     * @param rawData The raw data from the data source to quantize
     * @param formattedData The quantized data is dumped into the vector.
     */
    template< class T >
    void _getQuantizedData( const T* rawData,
                            std::vector< T >& formattedData ) const;

    AllocMemoryUnitPtr _data;
    ConstVolumeDataSourcePtr _dataSource;
    uint32_t _gpuDataType;
};

}

#endif // _TextureDataObject_h_
