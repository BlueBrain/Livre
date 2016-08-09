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

#include <livre/lib/cache/DataObject.h>
#include <livre/lib/cache/DataCache.h>

#include <livre/core/data/LODNode.h>
#include <livre/core/data/MemoryUnit.h>
#include <livre/core/data/DataSource.h>
#include <livre/core/maths/Quantizer.h>

#include <eq/gl.h>

namespace livre
{

struct DataObject::Impl
{
public:

    Impl( DataObject& dataObject,
          DataCache& dataCache )
        : _data( new AllocMemoryUnit( ))
        , _dataObject( dataObject )
        , _dataCache( dataCache )
        , _dataSource( dataCache.getDataSource( ))
    {
        if( !load())
        {
            LBTHROW( CacheLoadException( dataObject.getId(),
                                         "Unable to construct data cache object" ));
        }
    }

    ~Impl()
    {
        _data->release();
    }

    size_t getDataSize() const
    {
        const LODNode& lodNode =
                _dataSource.getNode( NodeId( _dataObject.getId( )));

        const Vector3ui& overlap =
                    _dataSource.getVolumeInfo().overlap;
        const size_t elemSize =
                    _dataSource.getVolumeInfo().getBytesPerVoxel();
        const uint32_t compCount =
                    _dataSource.getVolumeInfo().compCount;
        const Vector3ui blockSize =
                    lodNode.getBlockSize() + overlap * 2;
        return blockSize.product() * elemSize * compCount;
    }

    size_t getSize() const
    {
        return getDataSize();
    }

    const void* getDataPtr() const
    {
        return _data->getData< void >();
    }

    template< class DEST_TYPE >
    bool readTextureData()
    {
        const NodeId nodeId( _dataObject.getId( ));
        ConstMemoryUnitPtr data = _dataSource.getData( nodeId );
        if( !data )
            return false;

        const void* rawData = data->getData< void >();
        _data->allocAndSetData( rawData, getDataSize( ));

        return true;
    }

    bool load()
    {
        const DataType dataType = _dataSource.getVolumeInfo().dataType;
        switch( dataType )
        {
            case DT_UINT8:
                return readTextureData< uint8_t >();
            case DT_UINT16:
                return readTextureData< uint16_t >();
            case DT_UINT32:
                return readTextureData< uint32_t >();
            case DT_INT8:
                return readTextureData< int8_t >();
            case DT_INT16:
                return readTextureData< int16_t >();
            case DT_INT32:
                return readTextureData< int32_t >();
            case DT_FLOAT:
                return readTextureData< float >();
            case DT_UNDEFINED:
                LBTHROW( std::runtime_error( "Undefined data type" ));
        }
        return false;
    }

    AllocMemoryUnitPtr _data;
    DataObject& _dataObject;
    DataCache& _dataCache;
    DataSource& _dataSource;
};

DataObject::DataObject( const CacheId& cacheId,
                        DataCache& dataCache )
    : CacheObject( cacheId )
    , _impl( new Impl( *this, dataCache ))
{}

DataObject::~DataObject()
{}

size_t DataObject::_getSize() const
{
    if( !_isValid( ))
        return 0;

    return _impl->getSize();
}

const void* DataObject::getDataPtr() const
{
    return _impl->getDataPtr();
}

}
