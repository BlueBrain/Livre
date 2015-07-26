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

#ifndef _DashProcessor_h_
#define _DashProcessor_h_

#include <dash/dash.h>
#include <livre/core/api.h>
#include <livre/core/Pipeline/Processor.h>
#include <livre/core/Dash/DashContextTrait.h>

namespace livre
{

/**
 * The Processor class is the base class of the processing step in a pipeline. It can listen
 * the input connections and can behave accordingly. It has input connections and output connections
 * ( \see Connection ). When underlying thread is started with start() method it starts the infinite loop.
 */
class DashProcessor : public Processor, public DashContextTrait
{
public:
    LIVRECORE_API DashProcessor();

protected:
    /**
     * Checks whether connection is valid.
     * @param connection connection id
     * @return False if connection is not valid
     */
    LIVRECORE_API bool onPreApply_( const uint32_t connection );

    /**
     * Checks whether connection is valid.
     * @param connection connection id
     * @return False if connection is not valid
     */
    LIVRECORE_API bool onPreCommit_( const uint32_t connection );

    /**
     * Is executed when the dash context is set.
     */
    LIVRECORE_API virtual void onSetDashContext_();

    /**
     * Initializes the dash context for thread.
     * @return True if thread initialization is successfull.
     */
    LIVRECORE_API virtual bool initializeThreadRun_( );
};

}

#endif // _DashProcessor_h_
