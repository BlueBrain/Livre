/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#ifndef _Promise_h_
#define _Promise_h_

#include <livre/core/pipeline/Future.h>
#include <livre/core/types.h>

namespace livre
{

class Promise
{
public:

    Promise( const PipeFilter& pipeFilter, const AsyncData& data );
    ~Promise();

    const std::string& getName() const;

    /**
     * @return sets the promise with data
     */
    void set( ConstPortDataPtr msg );

    /**
     * @return the future, that can be queried for data retrieval
     */
    Future getFuture() const;

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _Promise_h_

