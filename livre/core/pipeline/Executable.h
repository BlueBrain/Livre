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

namespace livre
{

/**
 * The Executable class is the base class for objects that can be executed by workers. It
 * provides extra pre/post conditions as futures to retrieve the current situation.
 *
 * According to given futures executors can decide on scheduling algorithms. Also the
 * input and are output futures can be queried for the results. @see PipeFilter and
 * @see Pipeline classes are based on this class.
 */
class Executable
{
public:

    Executable() {}
    virtual ~Executable() {}

    /**
     * Executes the executable
     */
    virtual void execute() = 0;

    /**
     * @return the output futures for querying the outputs of the executable.
     */
    virtual Futures getPostconditions() const { return Futures(); }

    /**
     * @return the input futures which the executable can be queried for the state or
     * data retrieval.
     */
    virtual Futures getPreconditions() const { return Futures(); }

    /**
     * Resets the executable
     */
    virtual void reset() {}

};

}

#endif // _Executable_h_

