/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <livre/lib/cache/HistogramObject.h>
#include <livre/lib/cache/DataObject.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/data/Histogram.h>
#include <livre/core/data/DataSource.h>

#include <eq/gl.h>

namespace livre
{
namespace
{
template< class SRC_TYPE >
void binData( const SRC_TYPE* rawData,
              uint64_t* dstData,
              const Vector3ui& blockSize,
              const Vector3ui& padding,
              const size_t compCount,
              const uint64_t scaleFactor,
              const size_t resolution )
{
    const Vector2f dataSourceRange( std::numeric_limits< SRC_TYPE >::min(),
                                    std::numeric_limits< SRC_TYPE >::max());
    const double binCount = resolution;
    const double minVal = dataSourceRange[ 0 ];
    const double maxVal = dataSourceRange[ 1 ];
    const double range = maxVal - minVal;

    const Vector3ui dataBlockSize = blockSize + padding * 2;
    for( size_t i = padding.x(); i < dataBlockSize.x() - padding.x(); ++i )
        for( size_t j = padding.y(); j < dataBlockSize.y() - padding.y(); ++j )
            for( size_t k = padding.z(); k < dataBlockSize.z() - padding.z(); ++k )
                for( size_t c = 0; c < compCount; ++c )
                {
                    const size_t index = compCount * i * dataBlockSize.y() * dataBlockSize.z() +
                                         compCount * j * dataBlockSize.z() +
                                         compCount * k + c;
                    const double data = rawData[ index ];
                    const size_t binIndex =
                            std::is_unsigned< SRC_TYPE >::value ?
                            ( data - minVal ) / range * ( binCount - 1 ):
                            (( data / range ) + 0.5f ) * ( binCount - 1 );
                    dstData[ binIndex ] += scaleFactor;
                }
}
}

struct HistogramObject::Impl
{
public:

    Impl( const CacheId& cacheId,
          const Cache& dataCache,
          const DataSource& dataSource )
    {
        if( !load( cacheId, dataCache, dataSource ))
            LBTHROW( CacheLoadException( cacheId, "Unable to construct histogram cache object" ));
    }

    size_t getSize() const
    {
        return sizeof( uint64_t ) * _histogram.getBinsSize();
    }

    bool load( const CacheId& cacheId,
               const Cache& dataCache,
               const DataSource& dataSource )
    {
        const VolumeInformation& volumeInfo = dataSource.getVolumeInfo();
        const size_t compCount = volumeInfo.compCount;

        if( compCount > 1 )
            LBTHROW( std::runtime_error( "Multiple channels are not supported "));

        ConstDataObjectPtr data =
                std::static_pointer_cast< const DataObject >(
                    dataCache.get( cacheId ));

        if( !data )
            return false;

        const void* rawData = data->getDataPtr();
        const LODNode& lodNode = dataSource.getNode( NodeId( cacheId ));
        const Vector3ui& voxelBox = lodNode.getVoxelBox().getSize();
        const Vector3ui& padding = volumeInfo.overlap;

        const uint64_t scaleFactor1d =
                1 << ( volumeInfo.rootNode.getDepth() - lodNode.getRefLevel() - 1 );

        const uint64_t scaleFactor = scaleFactor1d * scaleFactor1d * scaleFactor1d;

        const DataType dataType = volumeInfo.dataType;
        switch( dataType )
        {
           case DT_UINT8:
                binData( static_cast< const uint8_t* >( rawData ),
                         _histogram.getBins(), voxelBox, padding, compCount,
                         scaleFactor, _histogram.getBinsSize( ));
                break;
           case DT_UINT16:
                binData( static_cast< const uint16_t* >( rawData ),
                         _histogram.getBins(), voxelBox, padding, compCount,
                         scaleFactor, _histogram.getBinsSize( ));
                break;
           case DT_UINT32:
                binData( static_cast< const uint32_t* >( rawData ),
                         _histogram.getBins(), voxelBox, padding, compCount,
                         scaleFactor, _histogram.getBinsSize( ));
                break;
           case DT_INT8:
                binData( static_cast< const int8_t* >( rawData ),
                          _histogram.getBins(), voxelBox, padding, compCount,
                          scaleFactor, _histogram.getBinsSize( ));
                break;
           case DT_INT16:
                binData( static_cast< const int16_t* >( rawData ),
                         _histogram.getBins(), voxelBox, padding, compCount,
                         scaleFactor, _histogram.getBinsSize( ));
                break;
           case DT_INT32:
                binData( static_cast< const int32_t* >( rawData ),
                         _histogram.getBins(), voxelBox, padding, compCount,
                         scaleFactor, _histogram.getBinsSize( ));
                break;
           case DT_FLOAT:
           case DT_UNDEFINED:
           default:
           {
                LBTHROW( std::runtime_error( "Unimplemented data type." ));
           }
        }
        return true;
    }

    Histogram _histogram;
};

HistogramObject::HistogramObject( const CacheId& cacheId,
                                  const Cache& dataCache,
                                  const DataSource& dataSource )
    : CacheObject( cacheId )
    , _impl( new Impl( cacheId, dataCache, dataSource ))
{}

HistogramObject::~HistogramObject()
{}

size_t HistogramObject::getSize() const
{
    return _impl->getSize();
}

Histogram HistogramObject::getHistogram() const
{
    return _impl->_histogram;
}

}
