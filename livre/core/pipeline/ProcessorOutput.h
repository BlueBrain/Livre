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

#ifndef _ProcessorOutput_h_
#define _ProcessorOutput_h_

#include <livre/core/api.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * The CommitState enum states the situation of commit
 */
enum CommitState
{
    CS_COMMITED, //!< The commit is sent to the receiver.
    CS_NOCHANGE, //!< Nothing to commit.
    CS_BLOCKED //!< Did not commited.
};

/**
 * The ProcessorOutput class is to send commits to connections.
 */
class ProcessorOutput
{
public:
    LIVRECORE_API ProcessorOutput( Processor& processor );

    LIVRECORE_API virtual ~ProcessorOutput( );

    /**
     * Sends commits to the given \see Connection object.
     * @param outputConnection Output connection id.
     * @return The state of the commit.
     */
    LIVRECORE_API CommitState commit( const uint32_t outputConnection );

    /**
     * Blocks the connection, no operation is allowed after the blocking.
     * @param block Blocking flag, if true connection is blocked.
     * @param inputConnection Connection number.
     */
    LIVRECORE_API void setBlocked( const bool block, const uint32_t inputConnection );

    /**
     * @param inputConnection Connection number.
     * @return True if the connection is blocked.
     */
    LIVRECORE_API bool isBlocked( const uint32_t inputConnection ) const;

    /**
     * @return The number of connections.
     */
    size_t getNumberOfConnections() const;

protected:

    /**
     * \see ProcessorOutput::commit()
     */
    virtual CommitState commit_( const uint32_t outputConnection ) = 0;

    /**
     * Adds the basic information for connection id
     * @param outputConnection Connection id.
     */
    void addConnection_(  const uint32_t outputConnection );

    /**
     * Removes the basic information for connection id
     * @param outputConnection Connection id.
     */
    void removeConnection_(  const uint32_t outputConnection );

private:

    Processor& processor_;

    BoolMap blockedMap_;

};

}

#endif // _ProcessorOutput_h_
