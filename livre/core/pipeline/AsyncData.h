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

#ifndef _AsyncData_h_
#define _AsyncData_h_

#include <livre/core/types.h>

namespace livre
{

/**
 * Provides thread safe operations for setting/retrieving/querying
 * the data. The internal data has a name and data type.
 */
class AsyncData
{
public:

    /**
     * When connection is instantiated the data is not
     * set yet, so any get() call will block the retrieval
     * @param dataInfo name, data type pair
     */
    explicit AsyncData( const DataInfo& dataInfo );
    ~AsyncData();

    /**
     * @return the data type of the connection
     */
    std::type_index getDataType() const;

    /**
     * @return the name of the connection
     */
    std::string getName() const;

    /**
     * @param data sets the data
     * @throws std::runtime_error if data types does not match between data
     * and current async data
     */
    void set( PortDataPtr data );

    /**
     * @param dataType is the requested data type.
     * @return the data. If data is not set it will block.
     * @throws std::runtime_error if data types does not match between dataType
     * and current async data type
     */
    PortDataPtr get( const std::type_index& dataType ) const;

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

    friend bool waitForAny( const Futures& futures );

    AsyncData( const AsyncData& ) = delete;
    AsyncData& operator=( const AsyncData& ) = delete;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

/**
 * Waits for any futures to be ready. If there are already ready futures, the function returns
 * immediately.
 * @param futures that is waited to be ready
 * @return true if there are still not ready futures, false if all futures are ready.
 */
bool waitForAny( const Futures& futures );

}

#endif // _AsyncData_h_

