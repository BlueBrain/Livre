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
#include <livre/lib/cache/HistogramCache.h>
#include <livre/lib/cache/TextureDataCache.h>
#include <livre/lib/cache/TextureDataObject.h>

#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/data/Histogram.h>
#include <livre/core/data/DataSource.h>

#include <eq/gl.h>

namespace livre
{

struct HistogramObject::Impl
{
public:

    Impl( const HistogramObject& histogramObject,
          const HistogramCache& cache )
        : _histogramObject( histogramObject )
        , _dataCache( cache.getDataCache( ))
    {}

    bool isLoaded() const
    {
        return _histogram.get();
    }

    size_t getSize() const
    {
        return _histogram.get() ? sizeof( uint64_t ) * Histogram::binCount : 0;
    }

    void binData( const int8_t* rawData,
                  Histogram& histogram,
                  const Vector3ui& blockSize,
                  const Vector3ui& padding,
                  const size_t compCount,
                  const uint64_t scaleFactor) const
    {
        uint64_t* bins = histogram.getBins();
        for( size_t i = padding.x(); i < blockSize.x() - padding.x(); ++i )
            for( size_t j = padding.y(); j < blockSize.y() - padding.y(); ++j )
                for( size_t k = padding.z(); k < blockSize.z() -padding.z(); ++k )
                    for( size_t c = 0; c < compCount; ++c )
                    {
                        const size_t index = compCount * i * blockSize.y() * blockSize.z() +
                                             compCount * j * blockSize.z() +
                                             compCount * k + c;
                        bins[ 128 + rawData[ index ]] += scaleFactor;
                    }
    }

    void binData( const uint8_t* rawData,
                  Histogram& histogram,
                  const Vector3ui& blockSize,
                  const Vector3ui& padding,
                  const size_t compCount,
                  const uint64_t scaleFactor ) const
    {

        uint64_t* bins = histogram.getBins();
        for( size_t i = padding.x(); i < ( blockSize.x() - padding.x( )); ++i )
            for( size_t j = padding.y(); j < ( blockSize.y() - padding.y( )); ++j )
                for( size_t k = padding.z(); ( k < blockSize.z() -padding.z( )); ++k )
                    for( size_t c = 0; c < compCount; ++c )
                    {
                        const size_t index = compCount * i * blockSize.y() * blockSize.z() +
                                             compCount * j * blockSize.z() +
                                             compCount * k + c;
                        const uint8_t data = rawData[ index ];
                        bins[ data ] += scaleFactor;
                    }
    }

    template< class SRC_TYPE >
    void binData( const SRC_TYPE* rawData,
                  Histogram& histogram,
                  const Vector3ui& blockSize,
                  const Vector3ui& padding,
                  const size_t compCount,
                  const uint64_t scaleFactor ) const
    {
        uint64_t* bins = histogram.getBins();
        const float minVal = std::numeric_limits< SRC_TYPE >::min();
        const float maxVal = std::numeric_limits< SRC_TYPE >::max();
        const float range = maxVal - minVal;

        for( size_t i = padding.x(); i < blockSize.x() - padding.x(); ++i )
            for( size_t j = padding.y(); j < blockSize.y() - padding.y(); ++j )
                for( size_t k = padding.z(); k < blockSize.z() -padding.z(); ++k )
                    for( size_t c = 0; c < compCount; ++c )
                    {
                        const size_t index = compCount * i * blockSize.y() * blockSize.z() +
                                             compCount * j * blockSize.z() +
                                             compCount * k + c;
                        const float data = rawData[ index ];
                        const size_t binIndex =
                                std::is_unsigned< SRC_TYPE >::value ?
                                ( data - minVal ) / range * Histogram::binCount :
                                (( data / range ) + 0.5f ) * Histogram::binCount;
                        bins[ binIndex ] += scaleFactor;
                    }
    }

    bool load()
    {
        const VolumeInformation& volumeInfo = _dataCache.getDataSource().getVolumeInfo();
        const size_t compCount = volumeInfo.compCount;

        if( compCount > 1 )
            LBTHROW( std::runtime_error( "Multiple channels are not supported "));

        ConstTextureDataObjectPtr textureData =
                std::static_pointer_cast< const TextureDataObject >(
                    _dataCache.get( _histogramObject.getId( )));

        if( !textureData->isLoaded( ))
            return false;

        const void* rawData = textureData->getDataPtr();
        const LODNode& lodNode = _dataCache.getDataSource().getNode(
                    NodeId( _histogramObject.getId( )));
        const Vector3ui& voxelBox = lodNode.getVoxelBox().getSize();
        const Vector3ui& padding = volumeInfo.overlap;

        const uint64_t scaleFactor1d =
                1 << ( volumeInfo.rootNode.getDepth() - lodNode.getRefLevel() - 1 );

        const uint64_t scaleFactor = scaleFactor1d * scaleFactor1d * scaleFactor1d;

        const DataType dataType = volumeInfo.dataType;
        Histogram histogram;
        switch( dataType )
        {
           case DT_UINT8:
                binData( static_cast< const uint8_t* >( rawData ), histogram,
                         voxelBox, padding, compCount, scaleFactor  );
                break;
           case DT_UINT16:
                binData< uint16_t >( static_cast< const uint16_t* >( rawData ), histogram,
                                     voxelBox, padding, compCount, scaleFactor  );
                break;
           case DT_UINT32:
                binData< uint32_t >( static_cast< const uint32_t* >( rawData ), histogram,
                                     voxelBox, padding, compCount, scaleFactor  );
                break;
           case DT_INT8:
                binData( static_cast< const int8_t* >( rawData ), histogram,
                         voxelBox, padding, compCount, scaleFactor  );
                break;
           case DT_INT16:
                binData< int16_t >( static_cast< const int16_t* >( rawData ), histogram,
                                    voxelBox, padding, compCount, scaleFactor  );
                break;
           case DT_INT32:
                binData< int32_t >( static_cast< const int32_t* >( rawData ), histogram,
                                    voxelBox, padding, compCount, scaleFactor  );
                break;
           case DT_FLOAT32:
           case DT_FLOAT64:
           case DT_UNDEFINED:
           default:
           {
                LBTHROW( std::runtime_error( "Unimplemented data type." ));
           }
        }
         _histogram.reset( new Histogram( histogram ));
        return true;
    }

    void unload()
    {
        _histogram.reset();
    }

    const HistogramObject& _histogramObject;
    const TextureDataCache& _dataCache;
    std::unique_ptr< Histogram > _histogram;
};

HistogramObject::HistogramObject( const CacheId& cacheId,
                                  const HistogramCache& cache )
    : CacheObject( cacheId )
    , _impl( new Impl( *this, cache ))
{}

HistogramObject::~HistogramObject()
{}

bool HistogramObject::_isLoaded( ) const
{
    return _impl->isLoaded();
}

size_t HistogramObject::_getSize() const
{
    return _impl->getSize();
}

bool HistogramObject::_load()
{
    return _impl->load();
}

void HistogramObject::_unload()
{
    return _impl->unload();
}

Histogram HistogramObject::getHistogram() const
{
    if( !_impl->_histogram.get( ))
        return Histogram();

    return *_impl->_histogram;
}

}
