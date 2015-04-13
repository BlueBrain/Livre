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

#ifndef _Connection_h_
#define _Connection_h_

#include <dash/dash.h>

namespace livre
{

/**
 * The Receiver class is an abstract class to receive information from the source.
 */
template< class T >
class Receiver
{
public:

    /**
     * ~Receiver destrcutor
     */
    virtual ~Receiver() { }

    /**
     * @return The popped commit. It may be blocked or unblocked according to implementation.
     */
    virtual T pop( ) = 0;

    /**
     * @param result Returns all the elements from the source into the result array.
     */
    virtual void popAll( std::vector< T >& result  ) = 0;

    /**
     * Waits a certain amount of time for an pop event to happen, and returns the element to commit.
     * @param timeout The time to wait for a pop to happen.
     * @param commit The destination commit object.
     * @return True if an element is popped from the source in certain amount of time.
     */
    virtual bool timedPop( const unsigned timeout, T& commit ) = 0;

    /**
     * @return True if any commit arrived.
     */
    virtual bool hasData() const = 0;

    /**
     * Cleans the items waiting to be processed.
     */
    virtual void clear( ) = 0;
};

/**
 * The Sender class is an abstract class for sending commits.
 */
template< class T >
class Sender
{
public:

    virtual ~Sender( ) { }

    /**
     * Pushes a commit. According to implemetation it can be blocking or unblocking.
     * @param commit is the object to push.
     */
    virtual void push( T& commit ) = 0;

    /**
     * Pushes a list of commits. According to implemetation it can be blocking or unblocking.
     * @param commitList is the list of commits to push.
     */
    virtual void push( const std::vector< T >& commitList ) = 0;

    /**
     * Cleans the items waiting to be processed.
     */
    virtual void clear( ) = 0;
};

/**
* The Connection class keeps the shared queueing mechanism between sender and receiver. The connection is
* uni-directional from \see Sender to \see Receiver.
*/
template< class T >
class Connection
{
public:

    /**
     * @return The element from the queueing mechanism.
     */
    T pop( ) const
    {
        return receiver_->pop( );
    }

    /**
     * Returns all the elements from the queueing mechanism.
     * @param result Returns the elements from the queueing mechanism to the vector object. Oldest element has the
     * lowest index.
     */
    void popAll( std::vector< T >& result )
    {
        receiver_->popAll( result );
    }

    /**
     * Pops element from the queueing mechanism in a given time frame.
     * @param timeout Waiting period for element to appear in queue mechanism.
     * @param element Popped element from the queue.
     * @return False if no element appear in the time frame.
     */
    bool timedPop( const unsigned timeout, T& element )
    {
        return receiver_->timedPop( timeout, element );
    }

    /**
     * Pushes the latest change into sender objects queueing mechanism.
     * @param commit
     */
    void push( T& commit )
    {
        sender_->push( commit );
    }

    /**
     * Pushes the latest changes into sender objects queueing mechanism.
     * @param commits The vector holding the changes.
     */
    void push( const std::vector< T >& commits )
    {
        sender_->push( commits );
    }

    /**
     * @return True if receiver object has changes waiting.
     */
    bool hasData( ) const
    {
        return receiver_->hasData( );
    }

    /**
     * Cleans the queue.
     */
    void clear( )
    {
        sender_->clear();
    }

protected:

    Connection( )
    {
    }

    /**
     * Receiver end of connection.
     */
    boost::shared_ptr< Receiver< T > > receiver_;

    /**
     * Sender end of connection.
     */
    boost::shared_ptr< Sender< T > > sender_;
};

}


#endif // _Connection_h_
