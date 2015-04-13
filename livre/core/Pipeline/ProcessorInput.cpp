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

#include <livre/core/Pipeline/ProcessorInput.h>
#include <livre/core/Pipeline/Processor.h>

namespace livre
{

#define MAX_CONNECTIONS 64

ProcessorInput::ProcessorInput( Processor& processor )
    : processor_( processor )
{

}

ProcessorInput::~ProcessorInput()
{
}

bool ProcessorInput::dataWaitingOnInput( const uint32_t inputConnection ) const
{
    BoolMap::const_iterator it = blockedMap_.find( inputConnection );

    if( it->second )
        return false;

    return dataWaitingOnInput_( inputConnection );
}

bool ProcessorInput::apply( const uint32_t inputConnection )
{ 
    if( blockedMap_[ inputConnection ] )
        return false;

    if( !processor_.onPreApply_( inputConnection ) )
        return false;

    const bool ret = apply_( inputConnection );
    processor_.onPostApply_( inputConnection, ret );
    return ret;
}

bool ProcessorInput::applyAllTimed( const uint32_t inputConnection, const uint32_t timeMs )
{
    if( blockedMap_[ inputConnection ] )
        return false;

    if( !processor_.onPreApply_( inputConnection ) )
        return false;

    const bool ret =  applyAllTimed_( inputConnection, timeMs );
    processor_.onPostApply_( inputConnection, ret );
    return ret;
}

bool ProcessorInput::applyAll( const uint32_t inputConnection )
{
    if( blockedMap_[ inputConnection ] )
        return false;

    if( !processor_.onPreApply_( inputConnection ) )
        return false;

    const bool ret = applyAll_( inputConnection );
    processor_.onPostApply_( inputConnection, ret );
    return ret;
}

void ProcessorInput::setBlocked( const bool block, const uint32_t inputConnection /* =0 */)
{
    blockedMap_[ inputConnection ] = block;
}


bool ProcessorInput::isBlocked( const uint32_t inputConnection /* =0 */) const
{
    BoolMap::const_iterator it = blockedMap_.find( inputConnection );
    return it->second;
}

uint32_t ProcessorInput::getNumberOfConnections() const
{
    return blockedMap_.size();
}

void ProcessorInput::addConnection_( const uint32_t inputConnection )
{
    blockedMap_[ inputConnection ] = false;
}

void ProcessorInput::removeConnection_( const uint32_t inputConnection )
{
    blockedMap_.erase( inputConnection );
}

}
