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

#ifndef _DashContextTrait_h_
#define _DashContextTrait_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/dashTypes.h>

namespace livre
{

/**
 * DashContextTrait class is used to give dash abilities to the classes.
 */
class DashContextTrait
{
public:
    /**
     * Sets the dash context.
     * @param dashContextPtr context ptr.
     */
    LIVRECORE_API void setDashContext( DashContextPtr dashContextPtr );

    /**
     * @return The dash context.
     */
    LIVRECORE_API DashContextPtr getDashContext( );

    LIVRECORE_API virtual ~DashContextTrait() {}

private:

    /**
     * Is called when the dash context is set.
     */
    virtual void onSetDashContext_( ) {}

    DashContextPtr dashContextPtr_;

};


}

#endif // _Processor_h_
