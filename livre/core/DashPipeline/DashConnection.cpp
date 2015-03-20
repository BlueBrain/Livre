/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
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

#include <livre/core/DashPipeline/DashConnection.h>

namespace livre
{

class DashReceiver: public Receiver< dash::Commit >
{
public:

    DashReceiver( lunchbox::MTQueue< dash::Commit >& queue ) : queue_( queue ) { }
    virtual dash::Commit pop( ) { return queue_.pop(); }
    virtual void popAll( std::vector< dash::Commit >& result  ) { queue_.tryPop( queue_.getSize( ), result ); }
    virtual bool timedPop( const unsigned timeout, dash::Commit& commit ) { return queue_.timedPop( timeout, commit ); }
    virtual bool hasData( ) const { return queue_.getSize() > 0; }
    virtual void clear( ) { queue_.clear(); }
protected:
    lunchbox::MTQueue< dash::Commit >& queue_;
};


class DashSender: public Sender< dash::Commit >
{
public:
    DashSender( lunchbox::MTQueue< dash::Commit >& queue ) : queue_( queue ) { }
    virtual void push( dash::Commit& commit ) { queue_.push( commit ); }
    virtual void push( const std::vector< dash::Commit >& commits ) { queue_.push( commits ); }
    virtual void clear( ) { queue_.clear(); }

protected:
    lunchbox::MTQueue< dash::Commit >& queue_;
};

DashConnection::DashConnection( const uint32_t maxSize )
{
    queue_.setMaxSize( maxSize );
    sender_.reset( new DashSender( queue_ ) );
    receiver_.reset( new DashReceiver( queue_ ) );
}

void DashConnection::setSourceContext( DashContextPtr contextPtr )
{
    sourceContextPtr_= contextPtr;
}

void DashConnection::setDestinationContext( DashContextPtr contextPtr )
{
    destinationContextPtr_= contextPtr;
}

}
