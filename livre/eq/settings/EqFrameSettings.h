
/* Copyright (c) 2006-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
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

#ifndef _EqFrameSettings_h_
#define _EqFrameSettings_h_

#include <livre/lib/types.h>
#include <livre/core/settings/FrameSettings.h>

#include <livre/core/mathTypes.h>

#include <eq/types.h>
#include <co/serializable.h>

namespace livre
{
/**
 * The FrameSettings class enables/disables info, statistics. Changes view and also toogles the
 * screen shot.
 */
class EqFrameSettings : public co::Serializable,
                        public FrameSettings
{
public:

    /** Constructor. */
    EqFrameSettings();

    /**
     * Set the current view id.
     * @param id The view id.
     */
    void setCurrentViewId( const eq::uint128_t& id );

    /**
     * @return Returns the current view id.
     */
    eq::uint128_t getCurrentViewId() const;

private:

    SignalledVariable< eq::uint128_t > _currentViewId;

    void serialize( co::DataOStream& os, const uint64_t dirtyBits ) final;
    void deserialize( co::DataIStream& is, const uint64_t dirtyBits ) final;
};
}

#endif // _EqFrameSettings_h_
