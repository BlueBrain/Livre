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

#include <dash/detail/commit.h>

#include <livre/core/dashpipeline/DashProcessorOutput.h>
#include <livre/core/dashpipeline/DashProcessor.h>
#include <livre/core/dashpipeline/DashConnection.h>

namespace livre
{

DashProcessorOutput::DashProcessorOutput( DashProcessor& processor )
    : ProcessorOutput( processor )
{
    setDashContext( processor.getDashContext() );
}

bool DashProcessorOutput::addConnection(const uint32_t connectionId, DashConnectionPtr connection )
{
    connectionMapModificationLock_.set();
    if( connectionMap_.find( connectionId ) != connectionMap_.end() )
    {
        connectionMapModificationLock_.unset();
        return false;
    }

    connectionMap_[ connectionId ] = connection;
    connection->setSourceContext( getDashContext() );
    addConnection_( connectionId );
    connectionMapModificationLock_.unset();
    return true;
}

bool DashProcessorOutput::removeConnection( const uint32_t connectionId )
{
    connectionMapModificationLock_.set();
    if( connectionMap_.find( connectionId ) != connectionMap_.end() )
    {
        connectionMapModificationLock_.unset();
        return false;
    }

    connectionMap_.erase( connectionId );
    removeConnection_( connectionId );
    connectionMapModificationLock_.unset();
    return true;
}

DashConnectionPtr DashProcessorOutput::getConnection( const uint32_t connectionId )
{
    if( connectionMap_.find( connectionId ) == connectionMap_.end() )
        return DashConnectionPtr();

    return connectionMap_[ connectionId ];
}

CommitState DashProcessorOutput::commit_( const uint32_t outputConnection )
{
    dash::Commit com = getDashContext()->commit( );
    CommitState  ret = CS_NOCHANGE;

    if( !com.getImpl()->empty() )
    {
        connectionMap_[ outputConnection ]->push( com );
        ret = CS_COMMITED;
    }

    return ret;
}

}
