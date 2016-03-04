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

/**
 * The AsyncData class provides thread safe operations on data setting/retrieval/querying
 * based on boost::shared_future ( boost::wait_for_any is not supported by std yet ).
 */
class AsyncData
{
public:

    /**
     * When connection is instantiated the data is not
     * set yet, so any get() call will block the retrieval
     * @param name of the connection
     * @param dataType type of the data
     */
    AsyncData( const std::string& name,
               const std::type_index& dataType );

    /**
     * @return the name of the connection
     */
    const std::string& getName() const;

    /**
     * @param data sets the data
     */
    void set( const ConstPortDataPtr& data );

    /**
     * @return the data. If data is not set it will block.
     */
    const ConstPortDataPtr& get() const;

    /**
     * @return true if data is set.
     */
    bool isReady() const;

    /**
     * Waits until data is set
     */
    void wait() const;

    /**
     * Resets the promise/future
     */
    void reset();

private:

    /**
     * Waits for any future to be ready.
     * @param futures queried to be ready.
     * @note this function needs internal access to the futures ( which is not exposed in the API ).
     * @return true if any new futures are ready or futures included are not busy.
     */
    friend bool waitForAny( const Futures& futures );

    AsyncData( const AsyncData& ) = delete;
    AsyncData& operator=( const AsyncData& ) = delete;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

class OutputPort
{

public:

    OutputPort( const PipeFilter& pipeFilter, const PortInfo& portInfo );
    ~OutputPort();

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
    PromisePtr getPromise() const;

    /**
     * Connects an output port to input port
     * @param inputPort input port
     */
    void connect( InputPort& inputPort );

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

