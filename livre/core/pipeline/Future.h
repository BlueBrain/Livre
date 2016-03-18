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

#ifndef _Future_h_
#define _Future_h_

#include <livre/core/pipeline/PortData.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * The Future class is similar to the std::future classes in functionality and it has additional
 * information for the name and data type. It provides thread safe methods to query and get the
 * data. Futures are retrieved from the @see Promise class.
 */
class Future
{
public:

    /**
     * @param data holds the thread safe data (query/retrieve).
     */
    Future( const AsyncData& data );
    ~Future();

    /**
     * @return name of the future
     */
    std::string getName() const;

    /**
     * Gets a shallow copy of the future with the given name
     */
    Future rename( const std::string& name ) const;

    /**
     * Gets the value with the given type T. If output is not set
     * this function will block.
     * @return the value.
     * @throw std::runtime_error when the data is not exact
     * type T
     */
    template< class T >
    const T& get() const { return _get<T>(); }

    /**
     * Waits until the data is ready.
     */
    void wait() const;

    /**
     * @return true if port is ready for data retrieval.
     */
    bool isReady() const;

    /**
     * @param future is the future to be checked with
     * @return true if both futures are same
     */
    bool operator==( const Future& future ) const { return _impl == future._impl; }

    /**
     * @param future is the future to be checked with
     * @return true if implementation address is smaller than the other implementation address
     */
    bool operator<( const Future& future ) const { return _impl.get() < future._impl.get(); }

private:

    friend bool livre::waitForAny( const Futures& future );

    const AsyncData& _getAsyncData() const;

    template< class T >
    const T& _get() const
    {
        const auto& dataPtr =
                std::static_pointer_cast< const PortDataT< T >>( _getPtr( getType< T >( )));

        return dataPtr->data;
    }

    PortDataPtr _getPtr( const std::type_index& dataType ) const;

    struct Impl;
    std::shared_ptr<Impl> _impl;
};

}

#endif // _Future_h_

