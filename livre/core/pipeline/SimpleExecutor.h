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

#include <livre/core/api.h>
#include <livre/core/pipeline/Executor.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * Provides a very basic implementation for the Executor
 * class. It has a thread pool for executing multiple executables
 * asynchronously.
 *
 * The submitted executables are scheduled to the worker threads,
 * by looking at the preconditions if they are satisfied.
 */
class SimpleExecutor : public Executor
{
public:

    /**
     * @param threadCount is number of worker threads
     * @param threadPoolName the threads are renamed with the given name
     * @param glContext if a gl context is provided, a new context will be created and
     * the worker threads will share the context with the given context
     */
    LIVRECORE_API SimpleExecutor( size_t threadCount,
                                  const std::string& threadPoolName = "Simple Executor",
                                  ConstGLContextPtr glContext = ConstGLContextPtr( ));

    LIVRECORE_API virtual ~SimpleExecutor();

    /** @copydoc Executor::schedule */
    LIVRECORE_API void schedule( ExecutablePtr executable ) final;

    /** @copydoc Executor::clear */
    LIVRECORE_API void clear() final;

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _SimpleExecutor_h_
