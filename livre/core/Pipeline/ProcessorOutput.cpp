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

#include <livre/core/Pipeline/ProcessorOutput.h>
#include <livre/core/Pipeline/Processor.h>

namespace livre
{

ProcessorOutput::ProcessorOutput( Processor& processor )
    : processor_( processor )
{

}

ProcessorOutput::~ProcessorOutput()
{
}

CommitState ProcessorOutput::commit( const uint32_t outputConnection )
{
    if( blockedMap_[ outputConnection ] )
        return CS_BLOCKED;

    if( !processor_.onPreCommit_( outputConnection ) )
        return CS_NOCHANGE;

    const CommitState ret = commit_( outputConnection );
    processor_.onPostCommit_( outputConnection, ret );
    return ret;

}

void ProcessorOutput::setBlocked( const bool block, const uint32_t outputConnection /* =0 */)
{
    blockedMap_[ outputConnection ] = block;
}

bool ProcessorOutput::isBlocked( const uint32_t outputConnection /* =0 */ ) const
{
    BoolMap::const_iterator it = blockedMap_.find( outputConnection );

    return it->second;
}

size_t ProcessorOutput::getNumberOfConnections() const
{
    return blockedMap_.size();
}

void ProcessorOutput::addConnection_( const uint32_t outputConnection )
{
    blockedMap_[ outputConnection ] = false;
}

void ProcessorOutput::removeConnection_( const uint32_t outputConnection )
{
    blockedMap_.erase( outputConnection );
}

}
