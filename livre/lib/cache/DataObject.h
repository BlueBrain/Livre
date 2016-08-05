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

#ifndef _DataObject_h_
#define _DataObject_h_

#include <livre/lib/types.h>

#include <livre/core/cache/CacheObject.h> // base class

namespace livre
{

/**
 * The DataObject class stores raw data from the volume data source.
 */
class DataObject : public CacheObject
{
public:
    ~DataObject();

    /** @return A pointer to the data or 0 if no data is loaded. */
    const void* getDataPtr() const;

private:
    friend class DataCache;

    /**
     * Constructor
     * @param cacheId is the unique identifier
     * @param dataCache the histogram source data is retrieved from data cache
     * @throws std::runtime_error when the data cache does not have the data for cache id
     */
    DataObject( const CacheId& cacheId, DataCache& dataCache );

    /** @copydoc livre::CacheObject::getSize */
    size_t _getSize() const final;

    struct Impl;
    std::unique_ptr<Impl> _impl;

};

}

#endif // _DataObject_h_
