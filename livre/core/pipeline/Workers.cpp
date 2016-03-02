/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <livre/core/pipeline/Workers.h>
#include <livre/core/pipeline/Filter.h>
#include <livre/core/pipeline/PortFutures.h>
#include <livre/core/pipeline/PipeFilter.h>
#include <livre/core/render/GLContext.h>

#include <lunchbox/mtQueue.h>

#include <boost/thread/thread.hpp>

namespace livre
{

struct Workers::Impl
{


    Impl( Workers& workers,
          size_t nThreads,
          GLContextPtr glContext )
        : _workers( workers )
        , _glContext( glContext )
    {
        for( size_t i = 0; i < nThreads; ++i )
            _threadGroup.create_thread( boost::bind( &Impl::execute,
                                                     this ));

    }

    void execute()
    {
        if( _glContext )
        {
            GLContextPtr context = _glContext->createContext();
            _glContext->shareContext( context );
            context->makeCurrent();
        }

        while( true )
        {
            ExecutablePtr executable = _workQueue.pop();
            if( !executable )
                break;

            executable->execute();
        }
    }

    ~Impl()
    {
        for( size_t i = 0; i < getSize(); ++i )
            _workQueue.push( ExecutablePtr());
        _threadGroup.join_all();
    }

    void submitWork( ExecutablePtr executable )
    {
        _workQueue.push( executable );
    }

    size_t getSize() const
    {
        return _threadGroup.size();
    }

    struct Work
    {


    };

    Workers& _workers;
    lunchbox::MTQueue< ExecutablePtr > _workQueue;
    boost::thread_group _threadGroup;
    GLContextPtr _glContext;
};

Workers::Workers( size_t nThreads,
                  GLContextPtr glContext )
    : _impl( new Workers::Impl( *this,
                                nThreads,
                                glContext ))
{}

Workers::~Workers()
{}

void Workers::execute( ExecutablePtr executable )
{
    _impl->submitWork( executable );
}

size_t Workers::getSize() const
{
    return _impl->getSize();
}

}
