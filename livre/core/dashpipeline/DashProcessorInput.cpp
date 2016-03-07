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

#include <livre/core/dashpipeline/DashProcessor.h>

#include <livre/core/dashpipeline/DashProcessorInput.h>
#include <livre/core/dashpipeline/DashConnection.h>

namespace livre
{

DashProcessorInput::DashProcessorInput( DashProcessor& processor )
    : ProcessorInput( processor )
{
    setDashContext( processor.getDashContext() );
}

bool DashProcessorInput::addConnection( const uint32_t connectionId, DashConnectionPtr connection )
{
    connectionMapModificationLock_.set();
    if( connectionMap_.find( connectionId ) != connectionMap_.end() )
    {
      connectionMapModificationLock_.unset();
      return false;
    }

    connectionMap_[ connectionId ] = connection;
    connection->setDestinationContext( getDashContext() );
    addConnection_( connectionId );
    connectionMapModificationLock_.unset();
    return true;
}

bool DashProcessorInput::removeConnection( const uint32_t connectionId )
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

DashConnectionPtr DashProcessorInput::getConnection( const uint32_t connectionId )
{
    if( connectionMap_.find( connectionId ) == connectionMap_.end() )
        return DashConnectionPtr();

    return connectionMap_[ connectionId ];
}

bool DashProcessorInput::dataWaitingOnInput_( const uint32_t inputConnection ) const
{
    DashConnectionMap::const_iterator it = connectionMap_.find( inputConnection );
    return it->second->hasData( );
}

bool DashProcessorInput::apply_( const uint32_t inputConnection )
{
    DashConnectionPtr connection =  connectionMap_[ inputConnection ];
    dash::Commit commit = connection->pop();
    if( commit.getImpl()->empty() )
        return false;
    getDashContext()->apply( commit );
    return true;
}

bool DashProcessorInput::applyAll_( const uint32_t inputConnection )
{
    DashConnectionPtr connection =  connectionMap_[ inputConnection ];
    dash::Commit commit = connection->pop( );

    std::vector< dash::Commit > commits;
    connection->popAll( commits );

    if( commits.empty() &&  commit.getImpl()->empty() )
        return false;

    getDashContext()->apply( commit );

    for( std::vector< dash::Commit >::iterator it = commits.begin(); it != commits.end(); ++it )
    {
        getDashContext()->apply( *it );
    }
    return true;
}

bool DashProcessorInput::applyAllTimed_( const uint32_t inputConnection, const uint32_t timeMs )
{
    DashConnectionPtr connection =  connectionMap_[ inputConnection ];
    std::vector< dash::Commit > commits;
    dash::Commit commit;
    if( connection->timedPop( timeMs, commit ) )
    {
        if( !commit.getImpl()->empty() )
        {
           connection->popAll( commits );
        }
    }

    if( !commit.getImpl()->empty() )
         getDashContext()->apply( commit );

    if( commits.empty() )
        return false;

    for( std::vector< dash::Commit >::iterator it = commits.begin(); it != commits.end(); ++it )
    {
        getDashContext()->apply( *it );
    }

    return true;
}


}
