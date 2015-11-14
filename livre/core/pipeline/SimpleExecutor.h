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

#ifndef _SimpleExecutor_h_
#define _SimpleExecutor_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/Executor.h>

namespace livre
{

/**
 * SimpleExecutor class provides the simplest implementation
 * for the @see Executor class. The submitted pipelines are
 * pushed to the worker threads. The pipeline submission
 * is thread safe.
 *
 * The scheduler implemented in this class, just pushes the
 * executables without re-ordering. This may mean that if
 * all workers are blocked, the processing can suspend.
 *
 */
class SimpleExecutor : public Executor
{
public:

    /**
     * @param workers are thread pools that executes individual
     * executables ( PipeFilter, Pipeline )
     */
    explicit SimpleExecutor( WorkersPtr workers );
    virtual ~SimpleExecutor();

    /**
     * @copydoc Executor::submit( PipelinePtr pipeline )
     */
    void submit( PipelinePtr pipeline ) final;

    /**
     * @copydoc Executor::clear()
     */
    void clear() final;

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _Executore_h_
