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

namespace livre
{

/**
 * Structure that holds the port information ( name, default value )
 */
struct PortInfo
{
    /**
     * @param portName_ is the port name
     * @param defaultData_ is the default data
     */
    template< class T >
    PortInfo( const std::string& portName_,
              const T& defaultData_ )
        : portName( portName_ )
        , defaultData( makePortDataPtr( std::move( defaultData_ )))
    {}

    std::string portName;
    ConstPortDataPtr defaultData;
};

/**
 * Helper function to create port info.
 * @param portName is the port name
 * @param defaultData is the default data
 */
template< class T >
PortInfo makePortInfo( const std::string& portname, const T& defaultData )
{
    return PortInfo( portname, defaultData );
}

/**
 * Helper function to add port info to the list of port infos.
 * @param portInfos is the list of port infos.
 * @param portName is the port name
 * @param defaultData is the default data
 */
template< class T >
void addPortInfo( PortInfos& portInfos,
                  const std::string& portname,
                  const T& defaultData )
{
    portInfos.push_back( std::move( PortInfo( portname, defaultData )));
}


}

#endif // _PortInfo_h_

