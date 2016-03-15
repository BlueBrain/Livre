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

#ifndef _SimpleExecutor_h_
#define _SimpleExecutor_h_

#include <livre/core/pipeline/Executor.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * SimpleExecutor class provides a very basic implementation
 * for the @see Executor class. It has a thread pool for
 * executing multiple executables asynchronously.
 *
 * The submitted executables are queued for the worker threads.
 * without any real scheduling.The pipeline submission is
 * thread safe.
 *
 * The scheduler implemented in this class, pushes the
 * executables without re-ordering. The executable whose
 * preconditions are satisfied is marked for execution
 * in the thread pool.
 *
 * SimpleExecutor executes executables in a push based flow.
 */

class SimpleExecutor : public Executor
{
public:

    /**
     * @param threadCount number of workers are thread pools that executes individual
     * executables ( PipeFilter, Pipeline )
     */
    explicit SimpleExecutor( size_t threadCount );
    virtual ~SimpleExecutor();

    /**
     * @copydoc Executor::clear()
     */
    void clear() final;

private:

    /**
     * @copydoc Executor::_schedule
     */
    void _schedule( const Executables& executables ) final;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _SimpleExecutor_h_
