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

#ifndef _CudaObject_h_
#define _CudaObject_h_

#include <livre/lib/api.h>
#include <livre/lib/types.h>

#include <livre/core/cache/CacheObject.h> // base class

namespace livre
{

/**
 * The DataObject class stores raw data from the volume data source.
 */
class CudaObject : public CacheObject
{
public:

    enum DataOrdering
    {
        DO_NONE,
        DO_MORTON,
        DO_HILBERT
    };

    /**
     * Constructor
     * @param cacheId is the unique identifier
     * @param dataSource the data source cache object is created from
     * @param curve data is re-ordered according to space curve
     * @throws CacheLoadException when the data cache does not have the data for cache id
     */
    LIVRE_API CudaObject( const CacheId& cacheId,
                          DataSource& dataSource,
                          DataOrdering order = DO_NONE );

    LIVRE_API ~CudaObject();

    /** @return A pointer to the data or 0 if no data is loaded. */
    LIVRE_API const void* getDataPtr() const;

    /** @copydoc livre::CacheObject::getSize */
    LIVRE_API size_t getSize() const final;

    /** @return the data ordering in memory */
    LIVRE_API DataOrdering getDataOrdering() const;

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;

};

}

#endif // _DataObject_h_
