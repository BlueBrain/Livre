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

#ifndef _DashProcessorInput_h_
#define _DashProcessorInput_h_

#include <livre/Lib/types.h>
#include <livre/core/dashTypes.h>
#include <livre/core/Pipeline/ProcessorInput.h>
#include <livre/core/Dash/DashContextTrait.h>

namespace livre
{

/**
 * The DashProcessorInput class is responsibe for receiving commits from many connections.
 */
class DashProcessorInput : public ProcessorInput, protected DashContextTrait
{

public:

    /**
     * @param processor Adds DashProcessorInput to the destination processor for connections.
     */
    DashProcessorInput( DashProcessor& processor );

    /**
     * Adds a connection.
     * @param connectionId Connection id.
     * @param connection The connection between processors.
     * @return True if the connection can be added with the id.
     */
    bool addConnection( const uint32_t connectionId, DashConnectionPtr connection );

    /**
     * Removes a connection.
     * @param connectionId Connection id.
     * @return True if connection exists and can be removed.
     */
    bool removeConnection( const uint32_t connectionId );

    /**
     * @param connectionId Connection id.
     * @return The connection. If id is not valid, connection is empty.
     */
    DashConnectionPtr getConnection( const uint32_t connectionId );

private:

    bool dataWaitingOnInput_( const uint32_t inputConnection ) const;
    bool apply_( const uint32_t inputConnection );
    bool applyAllTimed_( const uint32_t inputConnection, const uint32_t timeMs  );
    bool applyAll_( const uint32_t inputConnection );

    DashConnectionPtrMap connectionMap_;
    lunchbox::Lock connectionMapModificationLock_;
};

}

#endif // _DashProcessorInput_h_
