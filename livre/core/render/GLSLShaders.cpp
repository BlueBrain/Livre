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

void printShaderLog( const Handle shader )
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

void printProgramLog( const Handle program )
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
          const Strings& paths,
          const Strings& glslCodes LB_UNUSED,
          const unsigned shaderType )
{
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
            glNamedStringARB( GL_SHADER_INCLUDE_ARB, -1, paths[i].c_str(),
                             -1, glslCodes[i].c_str() );
            pathsChar[i] = paths[i].c_str();
        }
        glCompileShaderIncludeARB( (GLuint)handle, GLsizei(paths.size()),
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

    return ret;
}

int loadShaders( const ShaderData& shaderData )
{
    glGetError(); // reset
    LBASSERT( glCreateProgram );
    const Handle program = glCreateProgram();
    if( program == 0 )
    {
        LBDEBUG << "glCreateProgram failed" << std::endl;
        return error();
    }

    Handle vertexShader = 0;
    if( !shaderData.vShader.empty( ))
    {
        const int error = load( vertexShader, shaderData.vShader,
                                shaderData.paths, shaderData.glslCodes,
                                GL_VERTEX_SHADER );
        if( !vertexShader )
        {
            glDeleteProgram( program );
            return error;
        }
        glAttachShader( program, vertexShader );
    }

    Handle fragmentShader = 0;
    if(!shaderData.fShader.empty())
    {
        const int error = load( fragmentShader, shaderData.fShader,
                                shaderData.paths, shaderData.glslCodes,
                                GL_FRAGMENT_SHADER );
        if( !fragmentShader )
        {
            deleteShader( vertexShader );
            glDeleteProgram( program );
            return error;
        }
        glAttachShader( program, fragmentShader );
    }

    Handle geometryShader = 0;

    if(!shaderData.gShader.empty())
    {
        const int error = load( geometryShader, shaderData.gShader,
                                   shaderData.paths, shaderData.glslCodes,
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
}

GLSLShaders::GLSLShaders( const ShaderData& shaderData )
    : _program( loadShaders( shaderData ))
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
}
