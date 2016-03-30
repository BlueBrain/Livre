
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
#include <livre/core/pipeline/AsyncData.h>
#include <livre/core/pipeline/Future.h>
#include <livre/core/pipeline/Promise.h>

#include <livre/core/data/NodeId.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/render/GLContext.h>

#include <lunchbox/mtQueue.h>

#include <boost/thread.hpp>

#include <algorithm>
#include <iterator>

namespace livre
{

bool operator<( const Future& future1, const Future& future2 )
{
    return future1._impl.get() < future2._impl.get();
}

struct SimpleExecutor::Impl
{

    Impl( const size_t threadCount, GLContextPtr glContext )
        : _workers( threadCount, glContext )
        , _unlockPromise( DataInfo( "LoopUnlock", getType< bool >( )))
        , _workThread( boost::thread( boost::bind( &Impl::schedule, this )))
    {}

    ~Impl()
    {
        _mtWorkQueue.clear();
        _mtWorkQueue.push( 0 );
        {
            ScopedLock lock( _promiseReset );
             _unlockPromise.set( true );
        }
        _workThread.join();
    }

    void schedule()
    {
        std::set< Future > inputConditions = { _unlockPromise.getFuture() };
        Executables executables;
        while( true )
        {
            const Futures inputConds( inputConditions.begin(), inputConditions.end( ));
            waitForAny( inputConds );

            {
                ScopedLock lock( _promiseReset );
                if( _unlockPromise.getFuture( ).isReady( ))
                {
                    while( !_mtWorkQueue.empty( ))
                    {
                        Executable* exec = _mtWorkQueue.pop();
                        if( !exec )
                            return;

                        executables.push_back( exec );
                    }

                    inputConditions.erase( _unlockPromise.getFuture( ));
                    _unlockPromise.reset();
                    inputConditions.insert( _unlockPromise.getFuture( ));

                }
            }

            Executables::iterator it = executables.begin();
            while( it != executables.end( ))
            {
                Executable* executable = *it;
                const Futures& preConds = executable->getPreconditions();
                const FutureMap futureMap( preConds );
                if( futureMap.isReady( ))
                {
                    _workers.schedule( *executable );
                    it = executables.erase( it );
                    for( const auto& future: futureMap.getFutures( ))
                        inputConditions.erase( future );
                }
                else
                {
                    ++it;
                    inputConditions.insert( preConds.begin(), preConds.end( ));
                }
            }
        }
    }

    void clear()
    {
        _mtWorkQueue.clear();
    }

    void schedule( Executable& exec )
    {
        const bool wasEmpty = _mtWorkQueue.empty();
        _mtWorkQueue.push_back( &exec );
        ScopedLock lock( _promiseReset );
        if( wasEmpty )
            _unlockPromise.set( true );
    }

    lunchbox::MTQueue< Executable* > _mtWorkQueue;
    Workers _workers;
    Promise _unlockPromise;
    boost::mutex _promiseReset;
    boost::thread _workThread;
};

SimpleExecutor::SimpleExecutor( const size_t threadCount, GLContextPtr glContext )
    : _impl( new Impl( threadCount, glContext ))
{
}

SimpleExecutor::~SimpleExecutor( )
{}

void SimpleExecutor::clear()
{
    _impl->clear();
}

void SimpleExecutor::schedule( Executable& executable )
{
    _impl->schedule( executable );
}

}
