
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *               2013, Ahmet Bilgili  <ahmet.bilgili@epfl.ch>
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

#ifndef _EqRenderSettings_h_
#define _EqRenderSettings_h_

#include <livre/lib/types.h>
#include <livre/core/settings/RenderSettings.h>

#include <co/serializable.h>

namespace livre
{

class EqRenderSettings : public co::Serializable,
                         public RenderSettings
{
    /** The changed parts of the data since the last pack( ). */
    enum DirtyBits
    {
        DIRTY_COLORMAP = co::Serializable::DIRTY_CUSTOM << 0u,
        DIRTY_DEPTH = co::Serializable::DIRTY_CUSTOM << 1u,
        DIRTY_CLIPPLANES = co::Serializable::DIRTY_CUSTOM << 2u
    };

public:

    /** Constructor. */
    EqRenderSettings();

private:

    void serialize( co::DataOStream& os, const uint64_t dirtyBits ) final;
    void deserialize( co::DataIStream& is, const uint64_t dirtyBits ) final;
};
}

#endif // _EqRenderSettings_h_
