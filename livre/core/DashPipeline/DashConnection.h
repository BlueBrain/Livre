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

#ifndef _DashConnection_h_
#define _DashConnection_h_

#include <livre/core/types.h>
#include <livre/core/dashTypes.h>
#include <livre/core/Pipeline/Connection.h>

#include <lunchbox/mtQueue.h>

namespace livre
{

/**
 * The DashConnection class implements the connection between dash processors based on dash commits.
 */
class DashConnection : public Connection< dash::Commit >
{
public:

    /**
     * @param maxSize Maximum size of queue.
     */
    DashConnection( const uint32_t maxSize );

    /**
     * Sets the source context.
     * @param contextPtr Source context.
     */
    void setSourceContext( DashContextPtr contextPtr );

    /**
     * Sets the destination context.
     * @param contextPtr Destination context.
     */
    void setDestinationContext( DashContextPtr contextPtr );

private:

    DashContextPtr sourceContextPtr_;

    DashContextPtr destinationContextPtr_;

    lunchbox::MTQueue< dash::Commit > queue_;

};

}

#endif // _DashConnection_h_
