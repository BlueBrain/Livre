/* Copyright (c) 2016, Blue Brain Project / EPFL
 *                     bbp-open-source@googlegroups.com
 *                     Stefan.Eilemann@epfl.ch
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

#ifndef _EventHandler_h_
#define _EventHandler_h_

#include <livre/eq/types.h>

namespace livre
{
/** Handles application events for a Config. */
template< class C > class EventHandler : public C
{
public:
    template< class... ARGS > EventHandler( Config& config, ARGS... args );
    ~EventHandler();

    void init();

    bool handleEvent( eq::EventICommand command ) override;
    bool handleEvent( eq::EventType type, const eq::KeyEvent& ) override;
    bool handleEvent( eq::EventType type, const eq::PointerEvent& ) override;

private:
    class Impl;
    std::unique_ptr< Impl > _impl;
};

}

#endif // _EventHandler_h_
