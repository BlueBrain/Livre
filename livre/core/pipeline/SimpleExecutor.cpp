
/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#include <livre/core/pipeline/SimpleExecutor.h>

#include <livre/core/pipeline/Filter.h>
#include <livre/core/pipeline/Workers.h>
#include <livre/core/pipeline/Executable.h>
#include <livre/core/pipeline/FutureMap.h>

#include <livre/core/data/NodeId.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/render/GLContext.h>

#include <lunchbox/mtQueue.h>

#include <boost/thread.hpp>

namespace livre
{

struct SimpleExecutor::Impl
{

    Impl( const size_t threadCount )
        : _workThread( boost::thread( boost::bind( &Impl::schedule, this )))
        , _workers( threadCount )
    {}

    ~Impl()
    {
        _mtWorkQueue.clear();
        _mtWorkQueue.push( Executables( ));
        _workThread.join();
    }

    void schedule()
    {
        while( true )
        {
            Executables executables = _mtWorkQueue.pop();
            if( executables.empty() )
                break;

            Futures futures;

            do
            {
                futures.clear();
                for( const Executable& executable: executables )
                {
                    const Futures& inputFutures = executable.getPreconditions();
                    for( const Future& future: inputFutures )
                    {
                        if( !future.isReady( ))
                            futures.push_back( future );
                    }
                }

                Executables::iterator it = executables.begin();
                while( it != executables.end( ))
                {
                    Executable& executable = *it;
                    const FutureMap futureMap( executable.getPreconditions( ));
                    if( futureMap.isReady( ))
                    {
                        _workers.execute( executable );
                        it = executables.erase( it );
                    }
                    else
                        ++it;
                }

            } while( FutureMap( futures ).waitForAny( ));
        }
    }

    void clear()
    {
        _mtWorkQueue.clear();
    }

    void submit( Executables work )
    {
        _mtWorkQueue.push( work );
    }

    lunchbox::MTQueue< Executables > _mtWorkQueue;
    boost::thread _workThread;
    Workers _workers;
};

SimpleExecutor::SimpleExecutor( const size_t threadCount )
    : _impl( new Impl( threadCount ))
{
}

SimpleExecutor::~SimpleExecutor( )
{}

void SimpleExecutor::clear()
{
    _impl->clear();
}

void SimpleExecutor::_schedule( const Executables& executables )
{
    _impl->submit( executables );
}

}
