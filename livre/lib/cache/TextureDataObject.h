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

#ifndef _TextureDataObject_h_
#define _TextureDataObject_h_

#include <livre/lib/types.h>

#include <livre/core/cache/CacheObject.h> // base class

namespace livre
{

/**
 * The TextureDataObject class gets raw data from the volume data source and
 * stores the quantized/formatted data for the GPU.
 */
class TextureDataObject : public CacheObject
{
public:
    ~TextureDataObject();

    /** @return A pointer to the data or 0 if no data is loaded. */
    const void* getDataPtr() const;

    /** @copydoc livre::CacheObject::getSize */
    size_t getSize() const final;

private:
    friend class TextureDataCache;

    TextureDataObject( const CacheId& cacheId,
                       TextureDataCache& dataCache );

    bool _load() final;
    void _unload() final;
    bool _isLoaded() const final;

    struct Impl;
    std::unique_ptr<Impl> _impl;

};

}

#endif // _TextureDataObject_h_
