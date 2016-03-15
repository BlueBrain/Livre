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

#include <livre/core/pipeline/AsyncData.h>
#include <livre/core/pipeline/PortData.h>
#include <livre/core/pipeline/Future.h>

#include <boost/thread/future.hpp>

namespace livre
{

typedef boost::shared_future< PortDataPtr > ConstPortDataFuture;
typedef boost::promise< PortDataPtr > ConstPortDataPromise;
typedef std::vector< ConstPortDataFuture > ConstPortDataFutures;

namespace
{
    /* Boost wait_for_any implementation uses the internal locks from the futures.
       Therefore, the get/set/query operations on promises/futures causes deadlocks
       when used with wait_for_any( futurelist ) if futurelist is including the
       future to be queried. With below implementation no locking is needed between
       future operations. A higher granularity can be added for checking whether
       the list includes the future or not, but simply exiting the wait_for_any
       operation when lock is owned is simpler.
     */
    ReadWriteMutex waitForAnyLock;
}

struct AsyncData::Impl
{
    Impl( const DataInfo& dataInfo )
        : _future( _promise.get_future( ))
        , _dataInfo( dataInfo )
    {}

    ~Impl()
    {
        if( !isReady())
            _promise.set_value( PortDataPtr( ));
    }

    const PortDataPtr& get( const std::type_index& dataType ) const
    {
        ReadLock lock( waitForAnyLock );

        const PortDataPtr& data = _future.get();

        if( !data )
            LBTHROW( std::runtime_error( "Returns empty data" ));

        if( data->dataType != dataType )
            LBTHROW( std::runtime_error( "Types does not match on get value"));

        return data;
    }

    void set( const PortDataPtr& data )
    {
        if( data )
        {
            if( _dataInfo.second != data->dataType )
                LBTHROW( std::runtime_error( "Types does not match on set value"));
        }

        ReadLock lock( waitForAnyLock );
        if( !_future.is_ready( ))
            _promise.set_value( data );
    }

    bool isReady() const
    {
        ReadLock lock( waitForAnyLock );
        return _future.is_ready();
    }

    void wait() const
    {
        ReadLock lock( waitForAnyLock );
        _future.wait();
    }

    void reset()
    {
        ConstPortDataPromise newPromise;
        _promise.swap( newPromise );
        _future = _promise.get_future();
    }

    ConstPortDataPromise _promise;
    mutable ConstPortDataFuture _future;
    const DataInfo _dataInfo;
};

AsyncData::AsyncData( const DataInfo& dataInfo )
    :  _impl( new Impl( dataInfo ))
{}

const PortDataPtr& AsyncData::get( const std::type_index& dataType ) const
{
    return _impl->get( dataType );
}

AsyncData::~AsyncData()
{}

const std::type_index& AsyncData::getDataType() const
{
    return _impl->_dataInfo.second;
}

const std::string& AsyncData::getName() const
{
    return _impl->_dataInfo.first;
}

void AsyncData::set( const PortDataPtr& data )
{
    _impl->set( data );
}

bool AsyncData::isReady() const
{
    return _impl->isReady();
}

void AsyncData::wait() const
{
    _impl->wait();
}

void AsyncData::reset()
{
    _impl->reset();
}

bool waitForAny( const Futures& futures )
{
    if( futures.empty( ))
        return false;

    ConstPortDataFutures boostFutures;
    boostFutures.reserve( futures.size( ));
    for( const auto& future: futures )
        boostFutures.push_back( future._getAsyncData()._impl->_future );

    WriteLock lock( waitForAnyLock, boost::try_to_lock );
    if( !lock.owns_lock( ))
        return true;

    boost::wait_for_any( boostFutures.begin(), boostFutures.end( ));
    return true;
}

}
