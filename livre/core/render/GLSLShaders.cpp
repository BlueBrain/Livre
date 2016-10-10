/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 *                          Maxim Makhinya
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

#include "GLSLShaders.h"

#include <livre/core/render/GLContext.h>
#include <eq/gl.h>
#include <lunchbox/debug.h>
#include <fstream>

namespace livre
{
namespace
{

void printShaderLog( const GLSLShaders::Handle shader )
{
    GLint length = 0;
    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &length );
    if( length <= 1 )
    {
        LBERROR << "Shader compile failed, but log is empty" << std::endl;
        return;
    }
    std::string log;
    log.resize( length );

    glGetShaderInfoLog( shader, length, 0, &log[0] );
    LBERROR << "Shader error: " << log << std::endl;
}

void printProgramLog( const GLSLShaders::Handle program )
{
    GLint length = 0;
    glGetProgramiv( program, GL_INFO_LOG_LENGTH, &length );
    if( length <= 1 )
    {
        LBERROR << "Shader program link failed, but log is empty" << std::endl;
        return;
    }
    std::string log;
    log.resize( length );

    glGetProgramInfoLog( program, length, 0, &log[0] );
    LBERROR << "Program error: " << log << std::endl;
}


void deleteShader( const GLSLShaders::Handle shader )
{
    if( shader )
        glDeleteShader( shader );
}

int load( GLSLShaders::Handle& handle,
          const std::string& shader,
          const ShaderIncludes& shaderIncludes,
          const unsigned shaderType )
{
    const Strings& paths = shaderIncludes.paths;
    const Strings& includes = shaderIncludes.includes;

    handle = glCreateShader( shaderType );
    if( !handle )
    {
        LBDEBUG << "glCreateShader failed" << std::endl;
        return glGetError();
    }

    const char* cstr = shader.c_str();
    glShaderSource( handle, 1, &cstr, 0 );

    if( paths.empty( ))
        glCompileShader( handle );
    else
    {
        std::vector< const char* > pathsChar( paths.size( ));
        for( size_t i = 0; i < paths.size() ; i++ )
        {
            glNamedStringARB( GL_SHADER_INCLUDE_ARB, -1,
                              paths[ i ].c_str(),
                             -1,
                              includes[ i ].c_str( ));
            pathsChar[i] = paths[i].c_str();
        }
        glCompileShaderIncludeARB( (GLuint)handle,
                                   GLsizei( paths.size( )),
                                   &pathsChar[0], 0 );
    }
    const int ret = glGetError();

    GLint status;
    glGetShaderiv( handle, GL_COMPILE_STATUS, &status );
    if( status == GL_FALSE )
    {
        printShaderLog( handle );
        glDeleteShader( handle );
        handle = 0;
    }

    for( size_t i = 0; i < paths.size(); i++ )
        glDeleteNamedStringARB( -1, paths[i].c_str( ));

    if( ret != GL_NO_ERROR )
        LBTHROW( std::runtime_error( "Can't load glsl shaders" ));


    return ret;
}

int loadShaders( const Shaders& shaders,
                 const ShaderIncludes& shaderIncludes )
{
    glGetError(); // reset
    LBASSERT( glCreateProgram );
    const GLSLShaders::Handle program = glCreateProgram();
    if( program == 0 )
    {
        LBDEBUG << "glCreateProgram failed" << std::endl;
        return glGetError();
    }

    GLSLShaders::Handle vertexShader = 0;
    if( !shaders.vShader.empty( ))
    {
        const int error = load( vertexShader,
                                shaders.vShader,
                                shaderIncludes,
                                GL_VERTEX_SHADER );
        if( !vertexShader )
        {
            glDeleteProgram( program );
            return error;
        }
        glAttachShader( program, vertexShader );
    }

    GLSLShaders::Handle fragmentShader = 0;
    if( !shaders.fShader.empty( ))
    {
        const int error = load( fragmentShader,
                                shaders.fShader,
                                shaderIncludes,
                                GL_FRAGMENT_SHADER );
        if( !fragmentShader )
        {
            deleteShader( vertexShader );
            glDeleteProgram( program );
            return error;
        }
        glAttachShader( program, fragmentShader );
    }

    GLSLShaders::Handle geometryShader = 0;
    if( !shaders.gShader.empty( ))
    {
        const int error = load( geometryShader,
                                shaders.gShader,
                                shaderIncludes,
                                GL_GEOMETRY_SHADER );
        if( !geometryShader )
        {
            deleteShader( fragmentShader );
            deleteShader( vertexShader );
            glDeleteProgram( program );
            return error;
        }
        glAttachShader( program, geometryShader );
    }

    glLinkProgram( program );
    const int error = glGetError();

    GLint status;
    glGetProgramiv( program, GL_LINK_STATUS, &status );
    if( status == GL_FALSE )
    {
        printProgramLog( program );
        deleteShader( geometryShader );
        deleteShader( fragmentShader );
        deleteShader( vertexShader );
        glDeleteProgram( program );
        return error;
    }

    return program;
}

std::string readShaderFile( const Strings& resourceFolders,
                            const std::string& file )
{
    for( const auto& resourceFolder: resourceFolders )
    {
        std::ifstream shader( resourceFolder + "/" + file );
        if( !shader.is_open( ))
            continue;

        std::stringstream str;
        str << shader.rdbuf();
        shader.close();
        return str.str();
    }

    LBTHROW( std::runtime_error( file + " cannot be read" ));
}
}

GLSLShaders::GLSLShaders( const Shaders& shaders,
                          const ShaderIncludes& shaderIncludes )
    : _program( loadShaders( shaders, shaderIncludes ))
{
}

GLSLShaders::~GLSLShaders()
{
    glDeleteProgram( _program );
}

GLSLShaders::Handle GLSLShaders::getProgram() const
{
    return _program;
}

ShaderFiles::ShaderFiles( const Strings& resourceFolders,
                          const std::string& vShaderFile,
                          const std::string& fShaderFile,
                          const std::string& gShaderFile )
{
    if( !vShaderFile.empty( ))
        vShader = readShaderFile( resourceFolders, vShaderFile );

    if( !fShaderFile.empty( ))
        fShader = readShaderFile( resourceFolders, fShaderFile );

    if( !gShaderFile.empty( ))
        gShader = readShaderFile( resourceFolders, gShaderFile );
}

}
