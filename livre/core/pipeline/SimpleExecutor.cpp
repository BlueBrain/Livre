
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

struct SimpleExecutor::Impl
{

    Impl( const size_t threadCount )
        : _workThread( boost::thread( boost::bind( &Impl::schedule, this )))
        , _workers( threadCount )
        , _unlockPromise( DataInfo( "LoopUnlock", getType< bool >( )))
    {}

    ~Impl()
    {
        _mtWorkQueue.clear();
        _mtWorkQueue.push( 0 );
        _workThread.join();
    }

    void schedule()
    {
        std::set< Future > inputConditions;
        std::set< Future > outputConditions;

        while( true )
        {
            Executables executables;
            Executable* executable = _mtWorkQueue.pop();
            if( !executable )
                return;

            executables.push_back( executable );

            for( const Executable* exec: executables )
                for( const Future& future: exec->getPreconditions( ))
                    inputConditions.insert( future );

            Executables::iterator it = executables.begin();
            while( it != executables.end( ))
            {
                Executable* exec = *it;
                const FutureMap futureMap( exec->getPreconditions( ));
                if( futureMap.isReady( ))
                {
                    _workers.execute( *exec );
                    it = executables.erase( it );

                    for( const Future& future: exec->getPostconditions( ))
                        outputConditions.insert( future );

                    for( const Future& future: exec->getPreconditions( ))
                        inputConditions.erase( future );
                }
                else
                    ++it;
            }

            Futures intersection;
            std::set_intersection( outputConditions.begin(), outputConditions.end(),
                                   inputConditions.begin(), inputConditions.end(),
                                   std::back_inserter( intersection ));

            // Wait only for the futures where executed output conditions
            // executables intersects
            if( !intersection.empty( ))
            {
                FutureMap futureMap( intersection );
                futureMap.waitForAny();
            }

            // Add left over executables back to queue
            for( Executable* exec: executables )
                _mtWorkQueue.push( exec );

            FutureMap futureMap( intersection );
            futureMap.waitForAny();

            std::set< Future >::iterator itFuture = outputConditions.begin();
            while( itFuture != outputConditions.end( ))
            {
                const Future& future = *itFuture;
                if( future.isReady( ))
                    itFuture = outputConditions.erase( itFuture );
                else
                    ++itFuture;
            }
        }
    }

    void clear()
    {
        _mtWorkQueue.clear();
    }

    void schedule( Executable& exec )
    {
        _mtWorkQueue.push( &exec );
    }

    lunchbox::MTQueue< Executable* > _mtWorkQueue;
    boost::thread _workThread;
    Workers _workers;
    Promise _unlockPromise;
    boost::mutex _promiseReset;
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

void SimpleExecutor::schedule( Executable& executable )
{
    _impl->schedule( executable );
}

}
