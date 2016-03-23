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

#include <lunchbox/log.h>

#include <boost/thread/future.hpp>

namespace livre
{

typedef boost::shared_future< PortDataPtr > ConstPortDataFuture;
typedef boost::promise< PortDataPtr > ConstPortDataPromise;
typedef std::vector< ConstPortDataFuture > ConstPortDataFutures;

struct AsyncData::Impl
{
    Impl( const DataInfo& dataInfo )
        : _future( _promise.get_future( ))
        , _dataInfo( dataInfo )
    {}

    ~Impl()
    {
        try
        {
            set( PortDataPtr( ));
        }
        catch( const std::runtime_error& )
        {}
    }

    PortDataPtr get( const std::type_index& dataType ) const
    {
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

        try
        {
            _promise.set_value( data );
        }
        catch( const boost::promise_already_satisfied& )
        {
            LBTHROW( std::runtime_error( "Data only can be set once"));
        }
    }

    bool isReady() const
    {
        return _future.is_ready();
    }

    void wait() const
    {
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

PortDataPtr AsyncData::get( const std::type_index& dataType ) const
{
    return _impl->get( dataType );
}

AsyncData::~AsyncData()
{}

std::type_index AsyncData::getDataType() const
{
    return _impl->_dataInfo.second;
}

std::string AsyncData::getName() const
{
    return _impl->_dataInfo.first;
}

void AsyncData::set( PortDataPtr data )
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

void waitForAny( const Futures& futures )
{
    if( futures.empty( ))
        return;

    ConstPortDataFutures boostFutures;
    boostFutures.reserve( futures.size( ));
    for( const auto& future: futures )
        boostFutures.push_back( future._getAsyncData()._impl->_future );

    boost::wait_for_any( boostFutures.begin(), boostFutures.end( ));
}

}
