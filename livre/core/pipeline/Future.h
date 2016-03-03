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

class Future
{
public:

    Future( const PipeFilter& pipeFilter,
            const AsyncData& data );
    ~Future();

    /**
     * @return name of the future
     */
    const std::string& getName() const;

    /**
     * Gets the value with the given type T. If output is not set
     * this function will block.
     * @return the value.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    const T& get() const
    {
        typedef typename std::remove_const<T>::type UnconstT;
        const auto& dataPtr =
                boost::dynamic_pointer_cast< const PortDataT< UnconstT >>( _get( ));

        if( !dataPtr )
            LBTHROW( std::runtime_error( "Invalid data type" ));

        return dataPtr->data;
    }

    /**
     * Moves the value with the given type T. If output is not set
     * this function will block.
     * @return the value.
     * @throw std::runtime_error when the port data is not exact
     * type T
     */
    template< class T >
    const T&& move() const
    {
        typedef typename std::remove_const<T>::type UnconstT;
        const auto& dataPtr =
                boost::dynamic_pointer_cast< const PortDataT< UnconstT >>( _get( ));

        if( !dataPtr )
            LBTHROW( std::runtime_error( "Invalid data type" ));

        return std::move( dataPtr->data );
    }

    /**
     * Waits until the data is ready.
     */
    void wait() const;

    /**
     * @return true if port is ready for data retrieval.
     */
    bool isReady() const;

    /**
     * @return the pipe filter that future belongs to. If executor supports pull mode,
     * by getting futures, it will be possible to reach the source of the execution
     */
    const PipeFilter& getPipeFilter() const;

    /**
     * @return the async data implementation
     */
    const AsyncData& getAsyncData() const;

private:

    friend bool livre::waitForAny( const Futures& future );

    ConstPortDataPtr _get() const;

    struct Impl;
    std::shared_ptr<Impl> _impl;
};

}

#endif // _Future_h_

