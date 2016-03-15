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

#ifndef _OutputPort_h_
#define _OutputPort_h_

#include <livre/core/types.h>

namespace livre
{

class OutputPort
{

public:

    /**
     * @param dataInfo is the name and type information for the data.
     * @param pipeFilter is the reference to the pipefilter class which instantiates the
     * Promise.
     */
    OutputPort( const PipeFilter& pipeFilter,
                const DataInfo& dataInfo );
    ~OutputPort();

    OutputPort( OutputPort&& port );
    /**
     * @return name of the port
     */
    const std::string& getName() const;

    /**
     * @return data type of the port
     */
    const std::type_index& getDataType() const;

    /**
     * @return the promise, that data can be written to
     */
    Promise getPromise() const;

    /**
     * Connects an output port to input port
     * @param port input port
     */
    void connect( InputPort& port );

    /**
     * Resets the promise/future
     */
    void reset();

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

bool waitForAny( const Futures& futures );

}

#endif // _OutputPort_h_

