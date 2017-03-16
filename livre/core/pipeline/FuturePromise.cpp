/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
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

#include <livre/core/pipeline/FuturePromise.h>

#include <servus/uint128_t.h>

#include <boost/thread/future.hpp>

namespace livre
{
typedef boost::shared_future<PortDataPtr> PortDataFuture;
typedef boost::promise<PortDataPtr> PortDataPromise;
typedef std::vector<PortDataFuture> PortDataFutures;

struct Future::Impl
{
    Impl(const PortDataFuture& future, const std::string& name,
         const servus::uint128_t& uuid)
        : _name(name)
        , _future(future)
        , _uuid(uuid)
    {
    }

    const std::string& getName() const { return _name; }
    PortDataPtr get(const std::type_index& dataType) const
    {
        const PortDataPtr& data = _future.get();

        if (!data)
            LBTHROW(std::runtime_error("Returns empty data"));

        if (data->dataType != dataType)
            LBTHROW(std::runtime_error("Types does not match on get value"));

        return data;
    }

    bool isReady() const { return _future.is_ready(); }
    void wait() const { return _future.wait(); }
    std::string _name;
    mutable PortDataFuture _future;
    servus::uint128_t _uuid;
};

struct Promise::Impl
{
    Impl(const DataInfo& dataInfo)
        : _dataInfo(dataInfo)
        , _uuid(servus::make_UUID())
        , _futureImpl(new Future::Impl(PortDataFuture(_promise.get_future()),
                                       dataInfo.first, _uuid))
    {
    }

    const std::string& getName() const { return _dataInfo.first; }
    std::type_index getDataType() const { return _dataInfo.second; }
    void set(const PortDataPtr& data)
    {
        if (data)
        {
            if (_dataInfo.second != data->dataType)
                LBTHROW(
                    std::runtime_error("Types does not match on set value"));
        }

        try
        {
            _promise.set_value(data);
        }
        catch (const boost::promise_already_satisfied&)
        {
            LBTHROW(std::runtime_error("Data only can be set once"));
        }
    }

    void reset()
    {
        try
        {
            _promise.set_value(PortDataPtr());
        }
        catch (const boost::promise_already_satisfied&)
        {
        }

        PortDataPromise promise;
        _promise.swap(promise);
        _uuid = servus::make_UUID();
        _futureImpl->_future = _promise.get_future();
        _futureImpl->_uuid = _uuid;
    }

    void flush()
    {
        try
        {
            _promise.set_value(PortDataPtr());
        }
        catch (const boost::promise_already_satisfied&)
        {
        }
    }

    PortDataPromise _promise;
    const DataInfo _dataInfo;
    servus::uint128_t _uuid;
    std::shared_ptr<Future::Impl> _futureImpl;
};

Promise::Promise(const DataInfo& dataInfo)
    : _impl(new Promise::Impl(dataInfo))
{
}

Promise::~Promise()
{
}

std::type_index Promise::getDataType() const
{
    return _impl->getDataType();
}

const std::string& Promise::getName() const
{
    return _impl->getName();
}

void Promise::flush()
{
    _impl->flush();
}

Future Promise::getFuture() const
{
    const Future ret(*this);
    return Future(ret);
}

void Promise::reset()
{
    _impl->reset();
}

void Promise::_set(PortDataPtr data)
{
    _impl->set(data);
}

Future::Future(const Promise& promise)
    : _impl(promise._impl->_futureImpl)
{
}

Future::Future(const Future& future)
    : _impl(new Future::Impl(future._impl->_future, future.getName(),
                             future._impl->_uuid))
{
}

Future::Future(Future&& future)
    : _impl(std::move(future._impl))
{
}

Future::~Future()
{
}

const std::string& Future::getName() const
{
    return _impl->getName();
}

Future::Future(const Future& future, const std::string& name)
    : _impl(new Future::Impl(future._impl->_future, name, future._impl->_uuid))
{
}

void Future::wait() const
{
    return _impl->wait();
}

bool Future::isReady() const
{
    return _impl->isReady();
}

bool Future::operator==(const Future& future) const
{
    return _impl->_uuid == future._impl->_uuid;
}

const servus::uint128_t& Future::getId() const
{
    return _impl->_uuid;
}

PortDataPtr Future::_getPtr(const std::type_index& dataType) const
{
    return _impl->get(dataType);
}

void waitForAny(const Futures& futures)
{
    if (futures.empty())
        return;

    PortDataFutures boostFutures;
    boostFutures.reserve(futures.size());
    for (const auto& future : futures)
        boostFutures.push_back(future._impl->_future);

    boost::wait_for_any(boostFutures.begin(), boostFutures.end());
}
}
