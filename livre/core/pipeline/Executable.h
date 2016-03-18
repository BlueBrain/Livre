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

#ifndef _Executable_h_
#define _Executable_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/Future.h>
#include <livre/core/pipeline/Executor.h>

namespace livre
{

/**
 * Is the base class for execution. It provides methods for execution,
 * preconditions and postconditions for decision on scheduling.
 */
class Executable
{
public:

    /**
     * Executes the executable
     */
    virtual void execute() =  0;

    /**
     * Schedules the executable through an Executor
     * @param executor schedules the executable
     * @return the post conditions
     */
    Futures schedule( Executor& executor );

    /**
     * @return the output conditions for getting the outputs of the executable. The post
     * conditions has to be fullfilled by the execute() implementation ( at the end of
     * execution all futures should be ready )
     */
    virtual Futures getPostconditions() const = 0;

    /**
     * @return the input conditions which the executable can be queried for the state or
     * data retrieval.
     */
    virtual Futures getPreconditions() const = 0;

    /**
     * Resets the executable by setting all pre and post conditions to an clean state
     * ( The futures are not ready )
     */
    virtual void reset() {}

    virtual ~Executable();

protected:

    /**
     * @copydoc Executable::schedule
     */
    virtual void _schedule( Executor& executor );
};

}

#endif // _Executable_h_

