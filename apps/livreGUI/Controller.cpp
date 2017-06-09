/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Maxim Makhinya <maxmah@gmail.com>
 *                          David Steiner  <steiner@ifi.uzh.ch>
 *                          Fatih Erol
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

#include "Controller.h"

#include <zerobuf/Zerobuf.h>
#include <zeroeq/zeroeq.h>

#include <algorithm>
#include <thread>

#include <QTimer>

namespace livre
{
class Controller::Impl
{
public:
    Impl()
        : _subscriber()
    {
        _timer.connect(&_timer, &QTimer::timeout,
                       [this] { _subscriber.receive(0); });
        _timer.start(100);
    }

    bool publish(const ::zerobuf::Zerobuf& zerobuf)
    {
        return _publisher.publish(zerobuf);
    }

    bool subscribe(::zerobuf::Zerobuf& zerobuf)
    {
        return _subscriber.subscribe(zerobuf);
    }

    bool unsubscribe(const ::zerobuf::Zerobuf& zerobuf)
    {
        return _subscriber.unsubscribe(zerobuf);
    }

private:
    zeroeq::Publisher _publisher;
    zeroeq::Subscriber _subscriber;
    QTimer _timer;
};

Controller::Controller()
    : _impl(new Controller::Impl())
{
}

Controller::~Controller()
{
}

bool Controller::publish(const ::zerobuf::Zerobuf& zerobuf)
{
    return _impl->publish(zerobuf);
}

bool Controller::subscribe(::zerobuf::Zerobuf& zerobuf)
{
    return _impl->subscribe(zerobuf);
}

bool Controller::unsubscribe(const ::zerobuf::Zerobuf& zerobuf)
{
    return _impl->unsubscribe(zerobuf);
}
}
