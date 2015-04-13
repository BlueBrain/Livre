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

#ifndef _ProcessorInput_h_
#define _ProcessorInput_h_

#include <livre/core/types.h>

namespace livre
{

/**
 * The ProcessorInput class provides abstract methods for application of commits to the context. The derived
 * class should fill the methods according to its \see Connection object type.
 * @todo Decision of abstraction of Commits when needed. Therefore, moving the connection list to the ProcessorInput
 * class.
 */
class ProcessorInput
{
public:

    /**
     * @param processor Processor, which the input belonging to.
     */
    ProcessorInput( Processor& processor );

    virtual ~ProcessorInput();

    /**
     * @param inputConnection Connection number.
     * @return True if data is waiting on input, false if connection is blocked.
     */
    bool dataWaitingOnInput( const uint32_t inputConnection ) const;

    /**
     * Applies the oldest and single change to the context. Blocks until a commit arrives.
     * @param inputConnection Connection number.
     * @return True if apply is successful. If result is false, either connection is blocked or commit is
     * empty.
     */
    bool apply( const uint32_t inputConnection );

    /**
     * Applies all the changes to the context. Blocks until a commit arrives.
     * @param inputConnection inputConnection Connection number.
     * @return True if apply is successful. If result is false, either connection is blocked or commit is
     * empty.
     */
    bool applyAll( const uint32_t inputConnection );

    /**
     * Applies all the changes to the context. Waits for an commit to arrive
     * for a certain amount of time, if no commit is in the \see connection. If there are already commits, returns
     * immediately.
     * @param inputConnection Connection number.
     * @param timeMs Time to wait.
     * @return True if apply is successful. If result is false, either connection is blocked, commit is
     * empty or waiting time is over without any activity.
     */
    bool applyAllTimed( const uint32_t inputConnection, const uint32_t timeMs );

    /**
     * Blocks the connection, no operation is allowed after the blocking.
     * @param block Blocking flag, if true connection is blocked.
     * @param inputConnection Connection number.
     */
    void setBlocked( const bool block, const uint32_t inputConnection );

    /**
     * @param inputConnection Connection number.
     * @return True if the connection is blocked.
     */
    bool isBlocked( const uint32_t inputConnection ) const;

    /**
     * @return The number of connections.
     */
    uint32_t getNumberOfConnections() const;

protected:

    /**
     * \see ProcessorInput::dataWaitingOnInput(). Derived class should implement.
     */
    virtual bool dataWaitingOnInput_( const uint32_t inputConnection ) const = 0;

    /**
     * \see ProcessorInput::apply(). Derived class should implement.
     */
    virtual bool apply_( const uint32_t inputConnection ) = 0;

    /**
     * \see ProcessorInput::applyAll(). Derived class should implement.
     */
    virtual bool applyAll_( const uint32_t inputConnection ) = 0;

    /**
     * \see ProcessorInput::applyAllTimed(). Derived class should implement.
     */
    virtual bool applyAllTimed_( const uint32_t inputConnection, const uint32_t timeMs ) = 0;

    /**
     * Adds the basic information for connection id.
     * @param inputConnection Connection id.
     */
    void addConnection_(  const uint32_t inputConnection );

    /**
     * Removes the basic information for connection id.
     * @param inputConnection Connection id.
     */
    void removeConnection_(  const uint32_t inputConnection );

private:

    Processor& processor_;

    BoolMap blockedMap_;
};

}

#endif // _ProcessorInput_h_
