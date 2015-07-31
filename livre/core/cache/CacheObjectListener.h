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

#ifndef _CacheObjectListener_h_
#define _CacheObjectListener_h_

#include <livre/core/types.h>

namespace livre
{
/**
 * The CacheObjectListener class holds the list of observers to notify.
 */
class CacheObjectListener
{
public:
    /**
     * @param observer The observer to notify.
     */
    void registerObserver( CacheObjectObserver* observer ) { observers_.insert( observer ); }

    /**
     * @param observer The observer to remove.
     */
    void unregisterObserver( CacheObjectObserver* observer ) { observers_.erase( observer ); }

    /**
     * @return The list of observers.
     */
    const std::set< CacheObjectObserver* >& getObservers( ) const { return observers_; }

protected:

    CacheObjectListener() {}

private:

    std::set< CacheObjectObserver* > observers_;
};

}

#endif // _CacheObjectListener_h_
