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

#ifndef _CachePolicy_h_
#define _CachePolicy_h_

#include <livre/core/types.h>

#include <livre/core/cache/Cache.h>
#include <livre/core/cache/CacheStatistics.h>

namespace livre
{
/**
 * The CachePolicy class to generate the order of the list of \see CacheObject s to be unloaded.
 */
class CachePolicy
{
    friend class Cache;

protected:

    CachePolicy()  {}

    virtual ~CachePolicy() {}

    /**
     * Checks whether the policy will be activated for current state of \see Cache.
     * @param cache The \see Cache to check for the policy.
     * @return True if the limitations are passed.
     */
    virtual bool willPolicyBeActivated( const Cache& cache ) const = 0;

    /**
     * Checks whether the policy satisfied after each unload.
     * @param cache The \see Cache to check for the policy.
     * @return True if the limitations are satisfied.
     */
    virtual bool isPolicySatisfied( const Cache& cache ) const = 0;

    /**
     * Modifies the list of the objects for unloading.
     * @param cache  The \see Cache to check for the policy.
     * @param cacheObjectList The incoming list of objects from \see Cache to order for unloading.
     * @param modifiedObjectList The output list to unload.
     */
    virtual void _apply( const Cache& cache,
                         const std::vector< CacheObject * >& cacheObjects,
                         std::vector< CacheObject * >& modifiedObjects ) = 0;



};

}

#endif // _CacheObject_h_
