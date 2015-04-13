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

#ifndef _RawDataObject_h_
#define _RawDataObject_h_

#include <livre/Lib/types.h>

#include <livre/core/Data/VolumeDataSource.h>
#include <livre/core/Data/LODNode.h>
#include <livre/core/Data/LODNodeTrait.h>
#include <livre/core/Data/MemoryUnit.h>
#include <livre/core/Cache/CacheObject.h>
#include <livre/core/Maths/Quantizer.h>

namespace livre
{

/**
 * The RawDataObject class holds the cached information/data for a block of raw Hiearachical Volume Data from the
 * data source.
 */
class RawDataObject : public CacheObject, public LODNodeTrait
{
    friend class RawDataCache; //!< Only the RawDataCache can generate a valid object.

public:

    RawDataObject( );

    virtual ~RawDataObject( );

    /**
     * @return The cache id. Currently cache id is the \see LODNode id.
     */
    CacheId getCacheID() const final;

    /**
     * @return True if both object is contructed for same \see LODNode id.
     */
    bool operator==( const RawDataObject& data ) const;

   /**
     * @return The data size on data source.
     */
    uint32_t getDataSize( ) const;

    /**
     * @return The current memory size of the object.
     */
    uint32_t getCacheSize( ) const;

    /**
     * @return The data source.
     */
    ConstVolumeDataSourcePtr getDataSource( ) const;

    /**
     * @param destData Returns the data into the vector. Destination data should be empty.
     */
    template< class T >
    void getData( std::vector< T >& destData ) const
    {
        data_->getData< T >( destData );
    }

    /**
     * @return A pointer to the data. If there is no data, NULL ptr is returned.
     */
    template< class T >
    const T* getDataPtr( ) const
    {
        getUnconst_()->updateLastUsedWithCurrentTime_( );
        const MemoryUnit* data = data_.get();
        return data->getData< T >( );
    }

    /**
     * Quantizes data into the given format with U.
     * @param formattedData The quantized data is dumped into the vector.
     */
    template< class U >
    void getQuantizedData( std::vector< U >& formattedData ) const
    {
        getUnconst_()->updateLastUsedWithCurrentTime_( );
        const uint32_t compCount = dataSourcePtr_->getVolumeInformation().compCount;
        const DataType dataType = dataSourcePtr_->getVolumeInformation().dataType;

        const uint32_t dataSize = getDataSize();

        formattedData.resize( dataSize );

        switch( dataType )
        {
           case DT_UINT8:
           {
                const Vector3f min( std::numeric_limits< uint8_t >::min( ) );
                const Vector3f max( std::numeric_limits< uint8_t >::max( ) );
                unsignedQuantize( data_->getData< uint8_t >(), &formattedData[ 0 ],
                       getDataSize(), compCount, min, max );
               break;
           }
           case DT_UINT16:
           {
               const Vector3f min( std::numeric_limits< uint16_t >::min( ) );
               const Vector3f max( std::numeric_limits< uint16_t >::max( ) );
               unsignedQuantize( data_->getData< uint16_t >(), &formattedData[ 0 ], getDataSize(),
                       compCount, min, max );
               break;
           }
           case DT_UINT32:
           {
              const Vector3f min( std::numeric_limits< uint32_t >::min( ) );
              const Vector3f max( std::numeric_limits< uint32_t >::max( ) );
              unsignedQuantize( data_->getData< uint32_t >(), &formattedData[ 0 ], getDataSize(),
                       compCount, min, max );
               break;
           }
           case DT_INT8:
           {
               const Vector3f min( std::numeric_limits< int8_t >::min( ) );
               const Vector3f max( std::numeric_limits< int8_t >::max( ) );
               signedQuantize( data_->getData< int8_t >(), &formattedData[ 0 ], getDataSize(),
                       compCount, min, max );
               break;
           }
           case DT_INT16:
           {
               const Vector3f min( std::numeric_limits< int16_t >::min( ) );
               const Vector3f max( std::numeric_limits< int16_t >::max( ) );
               signedQuantize( data_->getData< int16_t >(), &formattedData[ 0 ], getDataSize(),
                       compCount, min, max);
               break;
           }
           case DT_INT32:
           {
               const Vector3f min( std::numeric_limits< int32_t >::min( ) );
               const Vector3f max( std::numeric_limits< int32_t >::max( ) );
               signedQuantize( data_->getData< int32_t >(), &formattedData[ 0 ], getDataSize(),
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

    /**
     * @return An empty data object ptr.
     */
    static RawDataObject* getEmptyPtr();

private:

    /**
     * @param dataSource Data source.
     * @param node Node information.
     */
    RawDataObject( VolumeDataSourcePtr dataSourcePtr,
                   ConstLODNodePtr lodNodePtr );

    bool load_( );
    void unload_( );

    bool isLoaded_( ) const;
    bool isValid_( ) const;

    VolumeDataSourcePtr dataSourcePtr_;
    MemoryUnitPtr data_;
};

}

#endif // _RawDataObject_h_
