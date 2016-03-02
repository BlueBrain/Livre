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

#ifndef _Executable_h_
#define _Executable_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/Future.h>

namespace livre
{

/**
 * The Executable class is the base class for objects that can be executed by workers.
 */
class Executable
{
public:

    Executable() {}
    /**
     * Executes the executable
     */
    virtual void execute() = 0;

    /**
     * @return the input futures which the executable provides to wait. The
     * names of the futures are unique.
     */
    virtual Futures getOutFutures() const { return Futures(); }

    /**
     * @return the input futures which the executable is waiting to proceed.
     * The names of the futures not necessarily unique. The inputs accepts
     * multiple outputs with the same name.
     */
    virtual Futures getConnectedInFutures() const { return Futures(); }

protected:

    virtual ~Executable() {}
};

}

#endif // _Executable_h_

