/**
 * Copyright (c) BBP/EPFL 2005-2015 Ahmet.Bilgili@epfl.ch
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

#ifndef _coTypes_h_
#define _coTypes_h_

#include <livre/lib/types.h>
#include <co/serializable.h>
#include <co/objectHandler.h>
#include <co/objectMap.h>

namespace livre
{

/**
 * SmartPtr definitions
 */
typedef boost::shared_ptr< co::Serializable > CoSerializablePtr;
typedef boost::shared_ptr< co::ObjectMap > CoObjectMapPtr;
typedef boost::shared_ptr< co::ObjectFactory > CoObjectFactoryPtr;

typedef boost::scoped_ptr< boost::unordered_map< uint32_t, CoSerializablePtr > > CoSerializableMapPtr;

/**
 * Map definitions
 */
typedef boost::unordered_map< uint32_t, CoSerializablePtr > CoSerializableMap;
typedef boost::unordered_map< uint32_t, lunchbox::uint128_t > UUIDMap;

}

#endif // _coTypes_h_
