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

#include <livre/core/Events/EventInfo.h>

#include <livre/Eq/types.h>

namespace livre
{

/**
 * The EqEventInfo struct is the event info payload for event mapper.
 */
struct EqEventInfo : public EventInfo
{
    /**
     * @param _config The equalizer config.
     * @param _configEvent The event genrated by Equalizer.
     */
    EqEventInfo(  Config* _config, const eq::ConfigEvent* _configEvent )
        : config( _config ), configEvent( _configEvent )
    {}

    Config *config;
    const eq::ConfigEvent* configEvent;
};

}
