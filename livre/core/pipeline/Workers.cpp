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

#include <livre/core/pipeline/Workers.h>
#include <livre/core/pipeline/Executable.h>
#include <livre/core/render/GLContext.h>

#include <lunchbox/mtQueue.h>

#include <boost/thread/thread.hpp>

namespace livre
{

struct Workers::Impl
{
    Impl( Workers& workers,
          const size_t nThreads,
          const ConstGLContextPtr& glContext )
        : _workers( workers )
        , _glContext( glContext )
    {
        for( size_t i = 0; i < nThreads; ++i )
            _threadGroup.create_thread( boost::bind( &Impl::execute,
                                                     this ));
    }

    void execute()
    {
        GLContextPtr context;
        if( _glContext )
        {
            context = _glContext->clone();
            context->share( *_glContext );
            context->makeCurrent();
        }

        while( true )
        {
            ExecutablePtr exec = _workQueue.pop();
            if( !exec )
                break;

            exec->execute();
        }

        if( context )
        {
            context->doneCurrent();
            context.reset();
        }
    }

    ~Impl()
    {
        for( size_t i = 0; i < getSize(); ++i )
            _workQueue.push( ExecutablePtr( ));
        _threadGroup.join_all();
        _glContext.reset();
    }


    void submitWork( ExecutablePtr executable )
    {
        _workQueue.pushFront( executable );
    }

    size_t getSize() const
    {
        return _threadGroup.size();
    }

    Workers& _workers;
    lunchbox::MTQueue< ExecutablePtr > _workQueue;
    boost::thread_group _threadGroup;
    ConstGLContextPtr _glContext;
};

Workers::Workers( const size_t nThreads,
                  ConstGLContextPtr glContext )
    : _impl( new Workers::Impl( *this,
                                nThreads,
                                glContext ))
{}

Workers::~Workers()
{}

void Workers::schedule( ExecutablePtr executable )
{
    _impl->submitWork( executable );
}

size_t Workers::getSize() const
{
    return _impl->getSize();
}

}
