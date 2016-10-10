/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Maxim Makhinya
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
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

#ifndef _GLSLShaders_h_
#define _GLSLShaders_h_

#include <livre/core/api.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * The ShaderData structure holds all the informations for the shader (strings
 * for vertex, geometry, fragment shader and strings for separate glsl code used
 * with glsl's include directive).
 */
struct ShaderData
{
    std::string vShader;
    std::string fShader;
    std::string gShader;
    Strings paths;
    Strings glslCodes;

    ShaderData( std::string shaderV = "",
                std::string shaderF = "",
                std::string shaderG = "",
                ,
                Strings codesGlsl = Strings())
        : vShader(shaderV)
        , fShader(shaderF)
        , gShader(shaderG)
        , paths(glslPaths)
        , glslCodes(codesGlsl)
    {}
};

/**
 * Loads vertex, fragment and geometry shaders with GLSL include directive
 * support.
 */
class GLSLShaders
{
public:

    typedef unsigned Handle;

   /**
    * Constructor
    * @param shaderData The data needed to compile the shader.
    * @throw std::runtime_error when shader can not be loaded
    */
    LIVRECORE_API GLSLShaders( const ShaderData& shaderData );

    LIVRECORE_API GLSLShaders( Strings glslPaths = Strings() );
    LIVRECORE_API ~GLSLShaders();

    /** @return The OpenGL handle of the shaders. */
    LIVRECORE_API Handle getProgram() const;

private:

    Handle _program;
};

}
#endif // _GLSLShaders_h_
