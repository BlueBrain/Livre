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

#include <livre/core/cache/Cache.h>
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

    Impl( const CacheId& cacheId, DataSource& dataSource )
    {
        if( !load( cacheId, dataSource ))
            LBTHROW( CacheLoadException( cacheId, "Unable to construct data cache object" ));
    }

    ~Impl()
    {}

    const void* getDataPtr() const
    {
        return _data->getData< void >();
    }

    template< class DEST_TYPE >
    bool readTextureData( const CacheId& cacheId, DataSource& dataSource )
    {
        const NodeId nodeId( cacheId );
        _data = dataSource.getData( nodeId );
        if( !_data )
            return false;
        return true;
    }

    bool load( const CacheId& cacheId, DataSource& dataSource )
    {
        const DataType dataType = dataSource.getVolumeInfo().dataType;
        switch( dataType )
        {
            case DT_UINT8:
                return readTextureData< uint8_t >( cacheId, dataSource );
            case DT_UINT16:
                return readTextureData< uint16_t >( cacheId, dataSource );
            case DT_UINT32:
                return readTextureData< uint32_t >( cacheId, dataSource );
            case DT_INT8:
                return readTextureData< int8_t >( cacheId, dataSource );
            case DT_INT16:
                return readTextureData< int16_t >( cacheId, dataSource );
            case DT_INT32:
                return readTextureData< int32_t >( cacheId, dataSource );
            case DT_FLOAT:
                return readTextureData< float >( cacheId, dataSource );
            case DT_UNDEFINED:
                LBTHROW( std::runtime_error( "Undefined data type" ));
        }
        return false;
    }

    ConstMemoryUnitPtr _data;
};

DataObject::DataObject( const CacheId& cacheId, DataSource& dataSource )
    : CacheObject( cacheId )
    , _impl( new Impl( cacheId, dataSource ))
{}

DataObject::~DataObject()
{}

size_t DataObject::getSize() const
{
    return _impl->_data->getAllocSize();
}

const void* DataObject::getDataPtr() const
{
    return _impl->getDataPtr();
}

}
