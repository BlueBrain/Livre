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
#include <livre/core/render/GLContext.h>

#include <lunchbox/mtQueue.h>

#include <boost/thread/thread.hpp>

namespace livre
{

struct Workers::Impl
{


    Impl( Workers& workers,
          const size_t nThreads,
          const GLContextPtr& glContext )
        : _workers( workers )
        , _glContext( glContext )
    {
        for( size_t i = 0; i < nThreads; ++i )
            _threadGroup.create_thread( boost::bind( &Impl::execute,
                                                     this ));

    }

    struct Work
    {
        Work( const Executable& executable_ )
            : executable( executable_ )
        {}
        Executable executable;
    };

    typedef std::shared_ptr< Work > WorkPtr;

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
            WorkPtr work = _workQueue.pop();
            if( !work )
                break;

            work->executable.execute();
        }
    }

    ~Impl()
    {
        for( size_t i = 0; i < getSize(); ++i )
            _workQueue.push( WorkPtr());
        _threadGroup.join_all();
    }


    void submitWork( const Executable& executable )
    {
        _workQueue.push( WorkPtr( new Work( executable )));
    }

    size_t getSize() const
    {
        return _threadGroup.size();
    }

    Workers& _workers;
    lunchbox::MTQueue< WorkPtr > _workQueue;
    boost::thread_group _threadGroup;
    const GLContextPtr _glContext;
};

Workers::Workers( const size_t nThreads,
                  GLContextPtr glContext )
    : _impl( new Workers::Impl( *this,
                                nThreads,
                                glContext ))
{}

Workers::~Workers()
{}

void Workers::execute( const Executable& executable )
{
    _impl->submitWork( executable );
}

size_t Workers::getSize() const
{
    return _impl->getSize();
}

}
