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

#ifndef _TextureDataObject_h_
#define _TextureDataObject_h_

#include <livre/Lib/types.h>
#include <livre/Lib/Cache/RawDataObject.h>

#include <livre/core/Data/VolumeDataSource.h>
#include <livre/core/Data/LODNodeTrait.h>
#include <livre/core/Data/MemoryUnit.h>
#include <livre/core/Maths/Quantizer.h>

namespace livre
{

/**
 * The TextureDataObject class holds the quantized/formatted data for the GPU. It decreases the amount of
 * processing while data is dropped from GPU and re-uploaded to GPU.
 */
class TextureDataObject : public CacheObject, public LODNodeTrait
{
    friend class TextureDataCache;

public:

    TextureDataObject( );

    virtual ~TextureDataObject( );

    /**
     * @return The cache id.
     */
    virtual CacheId getCacheID() const;

    /**
     * @return True if two data object has the same cache id.
     */
    bool operator==( const TextureDataObject& data ) const;

   /**
     * @return The data size.
     */
    uint32_t getDataSize( ) const;

    /**
     * @return The memory usage.
     */
    virtual uint32_t getCacheSize( ) const;

    /**
     * @return The data source.
     */
    ConstVolumeDataSourcePtr getDataSource( ) const;

    /**
     * Sets the raw data source.
     * @param rawDataObject livre::RawDataObject.
     */
    void setRawData( ConstRawDataObjectPtr rawDataObject );

    /**
     * @return The GPU data type
     */
    uint32_t getGPUDataType( ) const;

    /**
     * @param destData Returns the data into the vector. Destination data should be empty.
     */
    template< class T >
    void getData( std::vector< T >& destData ) const
    {
        getUnconst_()->updateLastUsedWithCurrentTime_( );
        data_->getData< T >( destData );
    }

    /**
     * @return A pointer to the data. If there is no data, NULL ptr is returned.
     */
    template< class T >
    const T* getDataPtr( ) const
    {
        getUnconst_()->updateLastUsedWithCurrentTime_( );
        return data_->getData< T >();
    }

    /**
     * @return An empty data object ptr.
     */
    static TextureDataObject* getEmptyPtr();

private:

    TextureDataObject( uint32_t gpuDataType );

    bool load_( );
    void unload_( );

    bool isLoaded_( ) const;
    bool isValid_( ) const;

    const RawDataObject& getRawDataObject_( ) const;

    template< class T >
    void setTextureData_( bool quantize, bool normalize );

    ConstRawDataObjectPtr rawDataObject_;
    AllocMemoryUnitPtr data_;
    ConstVolumeDataSourcePtr dataSourcePtr_;
    uint32_t gpuDataType_;
};

}

#endif // _TextureDataObject_h_
