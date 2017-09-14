/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#pragma once

#include <livre/lib/api.h>
#include <livre/lib/types.h>
#include <livre/lib/zerobuf/volumeRendererParameters.h> // base class

#include <co/distributable.h> // base class

namespace livre
{
/**
 * Enhance the ZeroBuf VolumeRendererParameters for Collage serialization and
 * initialization from commandline parameters.
 */
class VolumeRendererParameters
    : public co::Distributable<v1::VolumeRendererParameters>
{
public:
    LIVRE_API VolumeRendererParameters();
    LIVRE_API VolumeRendererParameters(const int32_t argc,
                                       const char* const argv[]);
    static std::string getHelp();

private:
    options_description _getOptions() const;
};
}
