/*
 * Copyright (c) 2010-2016, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef _Error_h_
#define _Error_h_

#include <livre/eq/api.h>
#include <livre/eq/types.h>

namespace livre
{

/**
 * The Error enum defines errors produced by Livre.
 */
enum Error
{
    ERROR_LIVRE_ARB_SHADER_OBJECTS_MISSING = eq::ERROR_CUSTOM, //!< GL_ARB_shader_objects extension missing.
    ERROR_LIVRE_EXT_BLEND_FUNC_SEPARATE_MISSING, //!< GL_EXT_blend_func_separate extension missing.
    ERROR_LIVRE_ARB_MULTITEXTURE_MISSING, //!< GL_ARB_multitexture extension missing.
};

/**
 * Set up livre-specific error codes.
 */
LIVREEQ_API void initErrors();

/**
 * Clear livre-specific error codes.
 */
LIVREEQ_API void exitErrors();

}
#endif // _Error_h_
