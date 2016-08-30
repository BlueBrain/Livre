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
              Histogram& histogram,
              const Vector3ui& blockSize,
              const Vector3ui& padding,
              const size_t compCount,
              const uint64_t scaleFactor )
{
    const size_t binCount = histogram.getBins().size();
    uint64_t* dstData = histogram.getBins().data();

    if( std::is_integral< SRC_TYPE >::value )
    {
        histogram.setMin( std::numeric_limits< SRC_TYPE >::min( ));
        histogram.setMax( std::numeric_limits< SRC_TYPE >::max( ));
    }
    else
    {
        double minVal = histogram.getMin();
        double maxVal = histogram.getMax();

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
                        if( data < minVal )
                            minVal = data;
                        if( data > maxVal )
                            maxVal = data;
                    }

        histogram.setMin( minVal );
        histogram.setMax( maxVal );

        if(( maxVal - minVal ) == 0.0f )
        {
            histogram.getBins().clear();
            const size_t bins = ( dataBlockSize - padding ).product() * scaleFactor * compCount;
            histogram.getBins().push_back( bins );
            return;
        }
    }


    const Vector3ui dataBlockSize = blockSize + padding * 2;
    for( size_t i = padding.x(); i < dataBlockSize.x() - padding.x(); ++i )
        for( size_t j = padding.y(); j < dataBlockSize.y() - padding.y(); ++j )
            for( size_t k = padding.z(); k < dataBlockSize.z() - padding.z(); ++k )
                for( size_t c = 0; c < compCount; ++c )
                {
                    const size_t index = compCount * i * dataBlockSize.y() * dataBlockSize.z() +
                                         compCount * j * dataBlockSize.z() +
                                         compCount * k + c;

                    size_t binIndex = 0;
                    if( std::is_integral< SRC_TYPE >::value )
                    {
                        const size_t range =
                                std::lround( histogram.getMax() - histogram.getMin( )) + 1u;
                        const size_t perBinCount = range / binCount;
                        binIndex = ( rawData[ index ] - std::lround( histogram.getMin( )))
                                     / perBinCount;
                    }
                    else
                    {
                        const double data = rawData[ index ];
                        if( data == histogram.getMax( ))
                            binIndex = binCount - 1;
                        else
                        {
                            const double range = histogram.getMax() - histogram.getMin();
                            const double perBinCount = range / binCount;
                            binIndex = ( rawData[ index ] - histogram.getMin( )) / perBinCount;
                        }
                    }
                    dstData[ binIndex ] += scaleFactor;
                }
}
}

struct HistogramObject::Impl
{
public:

    Impl( const CacheId& cacheId,
          const Cache& dataCache,
          const DataSource& dataSource,
          const Vector2f& dataSourceRange )
        :  _size( 0 )
    {
       if( !load( cacheId, dataCache, dataSource, dataSourceRange ))
            LBTHROW( CacheLoadException( cacheId, "Unable to construct histogram cache object" ));
    }


    bool load( const CacheId& cacheId,
               const Cache& dataCache,
               const DataSource& dataSource,
               const Vector2f& dataSourceRange )
    {
        const VolumeInformation& volumeInfo = dataSource.getVolumeInfo();
        const size_t compCount = volumeInfo.compCount;

        if( compCount > 1 )
            LBTHROW( std::runtime_error( "Multiple channels are not supported "));

        ConstDataObjectPtr data = dataCache.get< DataObject >( cacheId );

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
                _histogram.resize( 256 );
                binData( static_cast< const uint8_t* >( rawData ),
                         _histogram, voxelBox, padding, compCount, scaleFactor );
                break;
           case DT_UINT16:
                _histogram.resize( 1024 );
                binData( static_cast< const uint16_t* >( rawData ),
                         _histogram, voxelBox, padding, compCount, scaleFactor );
                break;
           case DT_UINT32:
                _histogram.resize( 4096 );
                binData( static_cast< const uint32_t* >( rawData ),
                         _histogram, voxelBox, padding, compCount, scaleFactor );
                break;
           case DT_INT8:
                _histogram.resize( 256 );
                binData( static_cast< const int8_t* >( rawData ),
                         _histogram, voxelBox, padding, compCount, scaleFactor );
                break;
           case DT_INT16:
                _histogram.resize( 1024 );
                binData( static_cast< const int16_t* >( rawData ),
                         _histogram, voxelBox, padding, compCount, scaleFactor );
                break;
           case DT_INT32:
                _histogram.resize( 4096 );
                binData( static_cast< const int32_t* >( rawData ),
                         _histogram, voxelBox, padding, compCount, scaleFactor );
                break;
           case DT_FLOAT:
                _histogram.resize( 4096 );
                _histogram.setMin( dataSourceRange[ 0 ] );
                _histogram.setMax( dataSourceRange[ 1 ] );
                binData( static_cast< const float* >( rawData ),
                         _histogram, voxelBox, padding, compCount, scaleFactor );
                break;
           case DT_UNDEFINED:
           default:
           {
                LBTHROW( std::runtime_error( "Unimplemented data type." ));
           }
        }

        _size = sizeof( uint64_t ) * _histogram.getBins().size();
        return true;
    }

    Histogram _histogram;
    size_t _size;
};

HistogramObject::HistogramObject( const CacheId& cacheId,
                                  const Cache& dataCache,
                                  const DataSource& dataSource,
                                  const Vector2f& dataSourceRange )
    : CacheObject( cacheId )
    , _impl( new Impl( cacheId, dataCache, dataSource, dataSourceRange ))
{}

HistogramObject::~HistogramObject()
{}

size_t HistogramObject::getSize() const
{
    return _impl->_size;
}

const Histogram& HistogramObject::getHistogram() const
{
    return _impl->_histogram;
}

}
