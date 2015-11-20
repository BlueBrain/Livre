
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

#include <livre/core/pipeline/SimpleExecutor.h>

#include <livre/core/pipeline/Filter.h>
#include <livre/core/pipeline/Workers.h>
#include <livre/core/pipeline/Pipeline.h>
#include <livre/core/pipeline/PipeFilter.h>

#include <livre/core/data/NodeId.h>
#include <livre/core/data/LODNode.h>
#include <livre/core/render/GLContext.h>

#include <lunchbox/mtQueue.h>
#include <lunchbox/monitor.h>

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

namespace livre
{

struct SimpleExecutor::Impl
{

    Impl( WorkersPtr workers )
        : _workThread( boost::thread( boost::bind( &Impl::execute, this )))
        , _workers( workers )
    {}

    ~Impl()
    {
        _mtWorkQueue.clear();
        _mtWorkQueue.push( PipelinePtr( ));
        _workThread.join();
    }

    // Reschedules the work
    void execute()
    {
        while( true )
        {
            PipelinePtr pipeline = _mtWorkQueue.pop();
            if( !pipeline )
                break;

            BOOST_FOREACH( ExecutablePtr executable, pipeline->getExecutables())
                _workers->execute( executable );

        }
    }

    void clear()
    {
        _mtWorkQueue.clear();
    }

    void submit( PipelinePtr work )
    {
        _mtWorkQueue.push( work );
    }

    lunchbox::MTQueue< PipelinePtr > _mtWorkQueue;
    boost::thread _workThread;
    WorkersPtr _workers;
};

SimpleExecutor::SimpleExecutor( WorkersPtr workers )
    : _impl( new Impl( workers ))
{
}

SimpleExecutor::~SimpleExecutor( )
{
    
}

void SimpleExecutor::clear()
{
    _impl->clear();
}

void SimpleExecutor::execute( PipelinePtr pipeline )
{
    _impl->submit( pipeline );
}

}
