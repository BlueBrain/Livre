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

#ifndef _PortInfo_h_
#define _PortInfo_h_

#include <livre/core/types.h>

#include "PortType.h"

namespace livre
{

/**
 * Structure that holds the port information ( name, default value )
 */
struct PortInfo final : public PortType
{
    PortInfo( const std::string& name_,
              std::type_index& typeIndex )
        : PortType( typeIndex )
        , name( portName_ )
    {}

    const std::string name;
};

/**
 * Helper function to create port info.
 * @param name is the port name
 */
template< class T >
PortInfo makePortInfo( const std::string& name )
{
    return PortInfo( name, std::type_index( typeid( T )));
}

/**
 * Helper function to create port info.
 * @param name is the port name
 */
PortInfo makePortInfo( const std::string& name,
                       const std::type_index& typeIndex )
{
    return PortInfo( name, typeIndex );
}

/**
 * Helper function to add port info to the list of port infos.
 * @param portInfos is the list of port infos.
 * @param portName is the port name
 */
template< class T >
void addPortInfo( PortInfos& portInfos,
                  const std::string& portname )
{
    portInfos.push_back( makePortInfo< T >( portname ));
}


}

#endif // _PortInfo_h_

