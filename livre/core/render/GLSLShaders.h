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

    ShaderData( std::string shaderV = "", std::string shaderF = "",
                std::string shaderG = "", Strings glslPaths = Strings(),
                Strings codesGlsl = Strings( ))
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
    LIVRECORE_API GLSLShaders();
    LIVRECORE_API ~GLSLShaders();

    /**
     * Load shaders from strings with glsl's include directive support.
     * @param shaderData The data needed to compile the shader.
     * @return The OpenGL error, or GL_NO_ERROR on success
     */
    LIVRECORE_API int loadShaders(const ShaderData& shaderData);

    typedef unsigned Handle;

    /** @return The OpenGL handle of the shaders. */
    LIVRECORE_API Handle getProgram() const;

    /**
     * Check if an OpenGL extension is available.
     * @param extensionName The name of the extension to be checked.
     * @return True or false.
     */
    LIVRECORE_API bool checkOpenGLExtension( const std::string& extensionName );

private:
    Handle _program;

    int _load( GLSLShaders::Handle& handle, const std::string& shader,
               const Strings& paths, const Strings& glslCodes,
               unsigned shaderType );

    void _printShaderLog( Handle shader );
    void _printProgramLog( Handle program );

    int cleanupOnError_( Handle shader1, Handle shader2 = 0,
                         Handle shader3 = 0 );

    void _deleteShader( GLSLShaders::Handle shader );

    std::string readShaderFile_( const std::string &shaderFile ) const;
};

}
#endif // _GLSLShaders_h_
