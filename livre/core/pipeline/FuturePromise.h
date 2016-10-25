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

#ifndef _Promise_h_
#define _Promise_h_

#include <livre/core/api.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * Is similar to the std::promise classes in functionality and it has additional
 * information for the name and data type. It provides methods to set the data.
 */
class Promise
{
public:

    /**
     * @param dataInfo is the name and type information for the data.
     */
    LIVRECORE_API Promise( const DataInfo& dataInfo );
    LIVRECORE_API ~Promise();

    /**
     * @return the name of the connection
     */
    LIVRECORE_API std::type_index getDataType() const;

    /**
     * @return the name of the connection
     */
    LIVRECORE_API std::string getName() const;

    /**
     * Sets the port with the value.
     * @param value to be set
     * @throw std::runtime_error when the port data is not exact
     * type T or there is no such port name.
     */
    template< class T >
    void set( const T& value )
    {
        _set( std::make_shared< PortDataT< T >>( value ));
    }

    /**
     * Sets the promise with empty data if it is not set already
     */
    void flush();

    /**
     * @return the future, that can be queried for data retrieval.
     * @note reset() on the promise won't effect the future object.
     * If such behaviour is needed Future has a Future( const Promise& )
     * consttructor
     */
    LIVRECORE_API Future getFuture() const;

    /**
     * @resets the promise.( If related future is constructed using
     * Future( const Promise& ) constructor, future can be re-used
     * for different data )
     * The behavior of the function is undefined when multiple threads
     * execute query/get from future.
     */
    LIVRECORE_API void reset();

private:

    friend class Future;

    void _set( PortDataPtr data );

    struct Impl;
    std::shared_ptr<Impl> _impl;
};

/**
 * The Future class is similar to the std::future classes in functionality and it has additional
 * information for the name and data type. It provides thread safe methods to query and get the
 * data. Futures are retrieved from the  Promise class.
 */
class Future
{
public:

    ~Future();

    /**
     * Copy constructor
     * @param future to be copied
     */
    Future( const Future& future );

    /**
     * @return name of the future
     */
    std::string getName() const;

    /**
     * Constructs a shallow copy of the future with the given name
     */
    Future( const Future& future, const std::string& name );

    /**
     * Gets the value with the given type T. Blocks until data is
     * available.
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
     * @return true if both futures are belonging to same promise
     */
    bool operator==( const Future& future ) const;

    /**
     * @param future is the future to be checked with
     * @return true if both futures are belonging to different promise
     */
    bool operator!=( const Future& future ) const { return !(*this == future); }

    /**
     * @return the unique identifier for the future
     */
    const servus::uint128_t& getId() const;

    /**
     * Promise based construction is needed when reset() on the promise
     * affects the future directly.
     * Further copies from the future is not effected by reset()
     * on the promise.
     * @param promise that future is retrieved
     */
    Future( const Promise& promise );

private:

    friend class Promise;

    friend void waitForAny( const Futures& future );
    friend bool operator<( const Future& future1, const Future& future2 );

    template< class T >
    const T& _get() const
    {
        const auto dataPtr =
                std::static_pointer_cast< const PortDataT< T >>( _getPtr( getType< T >( )));

        return dataPtr->data;
    }

    PortDataPtr _getPtr( const std::type_index& dataType ) const;

    struct Impl;
    std::shared_ptr<Impl> _impl;
};

/**
 * Waits for any futures to be ready. If there are already ready futures, the function returns
 * immediately.
 * @param futures that is waited to be ready
 */
void waitForAny( const Futures& futures );

}

#endif // _Promise_h_

