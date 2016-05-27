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

#ifndef _Workers_h_
#define _Workers_h_

#include <livre/core/api.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * A simple thread pool
 */
class Workers
{
public:

    /**
     * Constructs a thread pool given the number of threads.
     * @param nThreads is the number of threads.
     * @param glContext if given, the threads can share this
     * context.
     */
    LIVRECORE_API Workers( size_t nThreads = 4,
                           ConstGLContextPtr glContext = ConstGLContextPtr( ));
    LIVRECORE_API ~Workers();

    /**
     * Submitted executable is scheduled to the execution
     * queue.
     * @param executable is executed by thread pool.
     */
    LIVRECORE_API void schedule( ExecutablePtr executable );

    /**
     * @return the size of thread pool.
     */
    LIVRECORE_API size_t getSize() const;

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;

};

}

#endif // _Workers_h_

