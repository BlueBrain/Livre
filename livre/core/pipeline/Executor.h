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

#ifndef _Executor_h_
#define _Executor_h_

#include <livre/core/types.h>

namespace livre
{

/**
 * Is base class for implementing different scheduling
 * algorithms for Executable objects. i.e. The IO or
 * CPU intensive algorithms can implement their own scheduling
 * algorithms. The push/pull scheduling algorithms can also
 * be implemented.
 */
class Executor
{
public:

    LIVRECORE_API virtual ~Executor() {}

    /**
     * Schedules the executables for execution. The deriving class should implement a
     * scheduling algorithm for the execution. ( i.e. there may be a work queue and
     * executables are selected from the work queue according to their pre-post
     * conditions
     * @param executable to schedule
     */
    virtual void schedule( ExecutablePtr executable ) = 0;

protected:

    /** Clears the executor ( i.e : Implementation can empty the work queue ) */
    LIVRECORE_API virtual void clear() {}

};

}

#endif // _Executor_h_
