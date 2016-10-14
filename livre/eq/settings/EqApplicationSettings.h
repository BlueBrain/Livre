/*
 * Copyright (c) 2016, ahmetbilgili@gmail.com
 *
 * This file is part of Livre <https://github.com/bilgili/Livre>
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

#ifndef _EqApplicationSettings_h_
#define _EqApplicationSettings_h_

#include <livre/lib/types.h>
#include <livre/core/settings/ApplicationSettings.h>

#include <co/serializable.h>

namespace livre
{
class EqApplicationSettings : public co::Serializable,
                              public livre::ApplicationSettings
{

public:

    /** Constructor. */
    EqApplicationSettings();

private:

    void serialize(   co::DataOStream& os, const uint64_t dirtyBits ) final;
    void deserialize( co::DataIStream& is, const uint64_t dirtyBits ) final;
};

}

#endif // _EqApplicationSettings_h_
