/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#include <eq/gl.h>
#include <lunchbox/debug.h>
#include <fstream>

namespace livre
{
GLSLShaders::GLSLShaders()
    : _program( 0 )
{}


GLSLShaders::~GLSLShaders()
{
    if( _program )
        glDeleteProgram( _program );
}


GLSLShaders::Handle GLSLShaders::getProgram() const
{
    return _program;
}

void GLSLShaders::_printShaderLog( const Handle shader )
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

void GLSLShaders::_printProgramLog( const Handle program )
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

int GLSLShaders::_load( GLSLShaders::Handle& handle, const std::string& shader,
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
#ifdef GL3_PROTOTYPES
        if( checkOpenGLExtension( "GL_ARB_shading_language_include" ) )
        {
            std::vector< const char* > pathsChar( paths.size() );
            for( size_t i = 0; i < paths.size() ; i++ )
            {
                glNamedStringARB( GL_SHADER_INCLUDE_ARB, -1, paths[i].c_str(),
                                 -1, glslCodes[i].c_str() );
                pathsChar[i] = paths[i].c_str();
            }
            glCompileShaderIncludeARB( (GLuint)handle, paths.size(),
                                       &pathsChar[0], 0 );
        }
        else
#endif
            LBERROR << "GL_ARB_shading_language_include not supported"
                    << std::endl;
    }
    const int ret = glGetError();

    GLint status;
    glGetShaderiv( handle, GL_COMPILE_STATUS, &status );
    if( status == GL_FALSE )
    {
        _printShaderLog( handle );
        glDeleteShader( handle );
        handle = 0;
    }

#ifdef GL3_PROTOTYPES
    for( size_t i = 0; i < paths.size(); i++ )
        glDeleteNamedStringARB( -1, paths[i].c_str( ));
#endif

    return ret;
}

std::string GLSLShaders::readShaderFile_( const std::string &shaderFile ) const
{
    std::ifstream shader( shaderFile.c_str() );
    std::stringstream shaderStr;
    shaderStr << shader.rdbuf();
    shader.close();
    return shaderStr.str();
}

namespace
{
void _deleteShader( const GLSLShaders::Handle shader )
{
    if( shader )
        glDeleteShader( shader );
}

int _glError( const int error = glGetError( ))
{
    if( error == GL_NO_ERROR )
        return EQ_UNKNOWN_GL_ERROR;
    return error;
}
}

int GLSLShaders::loadShaders( const ShaderData& shaderData )
{
    if( _program )
        return GL_NO_ERROR;

    glGetError(); // reset
    LBASSERT( glCreateProgram );
    const Handle program = glCreateProgram();
    if( program == 0 )
    {
        LBDEBUG << "glCreateProgram failed" << std::endl;
        return _glError();
    }

    Handle vertexShader = 0;
    if( !shaderData.vShader.empty() )
    {
        const int error = _load( vertexShader, shaderData.vShader,
                                 shaderData.paths, shaderData.glslCodes,
                                 GL_VERTEX_SHADER );
        if( !vertexShader )
        {
            glDeleteProgram( program );
            return _glError( error );
        }
        glAttachShader( program, vertexShader );
    }

    Handle fragmentShader = 0;
    if(!shaderData.fShader.empty())
    {
        const int error = _load( fragmentShader, shaderData.fShader,
                                 shaderData.paths, shaderData.glslCodes,
                                 GL_FRAGMENT_SHADER );
        if( !fragmentShader )
        {
            _deleteShader( vertexShader );
            glDeleteProgram( program );
            return _glError( error );
        }
        glAttachShader( program, fragmentShader );
    }

    Handle geometryShader = 0;
#ifdef GL3_PROTOTYPES
    if(!shaderData.gShader.empty())
    {
        const int error = _load( geometryShader, shaderData.gShader,
                                 shaderData.paths, shaderData.glslCodes,
                                 GL_GEOMETRY_SHADER );
        if( !geometryShader )
        {
            _deleteShader( fragmentShader );
            _deleteShader( vertexShader );
            glDeleteProgram( program );
            return _glError( error );
        }
        glAttachShader( program, geometryShader );
    }
#endif

    glLinkProgram( program );
    const int error = glGetError();

    GLint status;
    glGetProgramiv( program, GL_LINK_STATUS, &status );
    if( status == GL_FALSE )
    {
        _printProgramLog( program );
        _deleteShader( geometryShader );
        _deleteShader( fragmentShader );
        _deleteShader( vertexShader );
        glDeleteProgram( program );
        return _glError( error );
    }

    _program = program;
    return GL_NO_ERROR;
}

bool GLSLShaders::checkOpenGLExtension( const std::string& extensionName
                                        LB_UNUSED )
{
#ifdef GL3_PROTOTYPES
    std::string nameInList;
    GLint end;
    glGetIntegerv(GL_NUM_EXTENSIONS,&end);

    for ( GLint i = 0; i < end; ++i )
    {
        const GLubyte* namePtr= glGetStringi(GL_EXTENSIONS,i);
        nameInList = reinterpret_cast<const char*>(namePtr);
        if(extensionName==nameInList)
            return true;
    }
#endif
    return false;
}

}
