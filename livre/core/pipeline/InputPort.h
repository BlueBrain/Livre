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

#ifndef _InputPort_h_
#define _InputPort_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/pipeline/PortInfo.h>

namespace livre
{

/**
 * InputPort class implements the input port to a @PipeFilter. It
 * provides thread safe query and retrieval.
 */
class InputPort
{
public:

    /**
     * InputPort constructor based on port information
     * @param portInfo is the port information
     */
    explicit InputPort( const PortInfo& portInfo );
    ~InputPort();

    /**
     * @return name of the port
     */
    const std::string& getName() const;

    /**
     * @return data type of the port
     */
    const std::type_index& getDataType() const;

    /**
     * @return the number of the inputs to the port
     */
    size_t getSize() const;

    /**
     * @return Return all the input futures that port has.
     */
    const Futures& getFutures() const;

    /**
     * Connects an output port to input port
     * @param inputPort input port
     */
    void connect( const OutputPort& inputPort );

    /**
     * Disconnects an output port from input port
     * @param inputPort input port
     */
    void disconnect( const OutputPort& inputPort );

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // _InputPort_h_

