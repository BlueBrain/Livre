/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
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
#include <livre/core/data/DataSource.h>

namespace livre
{
struct DataObject::Impl
{
public:
    Impl(const CacheId& cacheId, DataSource& dataSource)
    {
        if (!load(cacheId, dataSource))
            LBTHROW(
                CacheLoadException(cacheId,
                                   "Unable to construct data cache object"));
    }

    ~Impl() {}
    const void* getDataPtr() const { return _data->getData<void>(); }
    bool load(const CacheId& cacheId, DataSource& dataSource)
    {
        const NodeId nodeId(cacheId);
        _data = dataSource.getData(nodeId);
        return !!_data;
    }

    ConstMemoryUnitPtr _data;
};

DataObject::DataObject(const CacheId& cacheId, DataSource& dataSource)
    : CacheObject(cacheId)
    , _impl(new Impl(cacheId, dataSource))
{
}

DataObject::~DataObject()
{
}

size_t DataObject::getSize() const
{
    return _impl->_data->getAllocSize();
}

const void* DataObject::getDataPtr() const
{
    return _impl->getDataPtr();
}
}
