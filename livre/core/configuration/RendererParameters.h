/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#ifndef _RendererParameters_h_
#define _RendererParameters_h_

#include <livre/core/configuration/Parameters.h>
#include <livre/core/api.h>
#include <livre/core/configuration/rendererParameters.h>

#include <co/distributable.h>

namespace livre
{

/**
 * Enhance the ZeroBuf RendererParameters for Collage serialization and
 * initialization from config file and/or commandline parameters.
 */
class RendererParameters
        : public co::Distributable< zerobuf::RendererParameters >,
          public Parameters
{
public:
    LIVRECORE_API RendererParameters();

protected:
    LIVRECORE_API void _initialize() final;
};

}

#endif // _RendererParameters_h_
