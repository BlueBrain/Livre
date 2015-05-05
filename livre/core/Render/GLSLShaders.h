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

#include <livre/core/types.h>

#ifdef GL3_PROTOTYPES
#  include <eq/client/gl.h>
#endif

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
    StringVector paths;
    StringVector glslCodes;

    ShaderData( std::string shaderV = "", std::string shaderF = "", std::string shaderG = "",
                StringVector glslPaths = StringVector(), StringVector codesGlsl = StringVector() )
        :vShader(shaderV), fShader(shaderF), gShader(shaderG), paths(glslPaths), glslCodes(codesGlsl) {}
};

/**
 * The GLSLShaders class is used to load vertex, fragment and geometry shaders with glsl's include directive support.
 */
class GLSLShaders
{
public:

    GLSLShaders();

    ~GLSLShaders();

    typedef unsigned Handle;

    /**
     * Load shaders from strings with glsl's include directive support.
     * @param shaderData The data needed to compile the shader.
     * @return The OpenGL error, or GL_NO_ERROR on success
     */
    int loadShaders(const ShaderData& shaderData);

    /**
     * @return The OpenGL handle of the shaders.
     */
    Handle getProgram() const;

    /**
     * Check if an OpenGL extension is available.
     * @param extensionName The name of the extension to be checked.
     * @return True or false.
     */
    static bool checkOpenGLExtension( const std::string& extensionName );

private:

    Handle loadShader_( const std::string& shader, const StringVector& paths,
                        const StringVector& glslCodes, const unsigned shaderType );

    void printLog_( Handle shader, const std::string &type );

    int cleanupOnError_( Handle shader1, Handle shader2 = 0, Handle shader3 = 0 );

    std::string readShaderFile_( const std::string &shaderFile ) const;

    Handle program_;

    bool shadersLoaded_;
};

}
#endif // _GLSLShaders_h_
