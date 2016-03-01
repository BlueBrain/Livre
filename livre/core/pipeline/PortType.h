/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
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

#ifndef _PortType_h_
#define _PortType_h_

#include <livre/core/types.h>

namespace livre
{

/**
 * PortType class is base class for keeping the track for types
 * by using the std::type_index.
 */
class PortType
{
public:

    /**
     * @return the type index
     */
    const std::type_index& getDataType() const
        { return _typeIndex; }

protected:

    /**
     * PortType constructor
     * @param typeIndex type index of the data of the
     * derived classes data type.
     */
    PortType( const std::type_index& typeIndex )
        : _typeIndex( typeIndex) {}
    virtual ~PortData() {}

private:
    std::type_index _typeIndex;
};

}

#endif // _PortData_h_

