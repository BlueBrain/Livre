
/* Copyright (c) 2010-2016, Stefan Eilemann <eile@eyescale.ch>
 *                          David Steiner   <steiner@ifi.uzh.ch>
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

#include <eq/global.h>

#include <livre/eq/Error.h>

#ifndef __APPLE__
#  include <eq/fabric/errorRegistry.h>
#  include <co/global.h>
#else
#  include <eq/fabric/errorRegistry.h>
#  include <co/global.h>
#endif

namespace livre
{

namespace
{

struct ErrorData
{
    const uint32_t code;
    const std::string text;
};

ErrorData errors_[] = {
    { ERROR_LIVRE_ARB_SHADER_OBJECTS_MISSING,      "GL_ARB_shader_objects extension missing" },
    { ERROR_LIVRE_EXT_BLEND_FUNC_SEPARATE_MISSING, "GL_EXT_blend_func_separate extension missing" },
    { ERROR_LIVRE_ARB_MULTITEXTURE_MISSING,        "GL_ARB_multitexture extension missing" },
    { 0, "" } // last!
};

}

void initErrors()
{
    eq::fabric::ErrorRegistry& registry = eq::Global::getErrorRegistry();

    for( size_t i=0; errors_[i].code != 0; ++i )
        registry.setString( errors_[i].code, errors_[i].text );
}

void exitErrors()
{
    eq::fabric::ErrorRegistry& registry = eq::Global::getErrorRegistry();

    for( size_t i=0; errors_[i].code != 0; ++i )
        registry.eraseString( errors_[i].code );
}

}
