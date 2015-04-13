/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Maxim Makhinya
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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
#include "gl.h"

#include <lunchbox/debug.h>
#include <fstream>

namespace livre
{

GLSLShaders::GLSLShaders()
    : program_( 0 )
    , shadersLoaded_( false )
{}


GLSLShaders::~GLSLShaders()
{
    if( !shadersLoaded_ )
        return;

    if( program_ )
        glDeleteObjectARB( program_ );
}


GLSLShaders::Handle GLSLShaders::getProgram() const
{
    return program_;
}

void GLSLShaders::printLog_( GLhandleARB shader, const std::string &type )
{
    GLint length;
    glGetObjectParameterivARB( shader, GL_OBJECT_INFO_LOG_LENGTH_ARB,
                               &length );
    if( length <= 1 )
        return;

    std::vector< GLcharARB > log;
    log.resize( length );

    GLint written;
    glGetInfoLogARB( shader, length, &written, &log[0] );
    LBERROR << "Shader error: " << type << &log[0] << std::endl;
}

GLSLShaders::Handle GLSLShaders::loadShader_(
    const std::string& shader, const StringVector& paths,
    const StringVector& glslCodes LB_UNUSED, const GLenum shaderType )
{
    GLhandleARB handle = glCreateShaderObjectARB( shaderType );
    std::vector<const char*> pathsChar( paths.size() );

    const char* cstr;
    cstr = shader.c_str();

    glShaderSourceARB( handle, 1, &cstr, 0 );

    if(paths.empty())
    {
        glCompileShaderARB( handle );
    }
    else
    {
#ifdef GL3_PROTOTYPES
        if( checkOpenGLExtension( "GL_ARB_shading_language_include" ) )
        {
            for( size_t i = 0; i < paths.size() ; i++ )
            {
                glNamedStringARB( GL_SHADER_INCLUDE_ARB, -1, paths[i].c_str(),
                                 -1, glslCodes[i].c_str() );
                pathsChar[i] = paths[i].c_str();
            }
            glCompileShaderIncludeARB( (GLuint)handle, paths.size(),
                                           &pathsChar[0], NULL );
        }
        else
#endif
        {
            LBERROR<<" Error: GL_ARB_shading_language_include is not supported. Sorry..." <<std::endl;
        }
    }

    GLint status;
    glGetObjectParameterivARB( handle,
                               GL_OBJECT_COMPILE_STATUS_ARB,
                               &status );
    if( status != GL_FALSE )
        return handle;

    printLog_( handle, "Compiling" );
    LBVERB << " Shader text -----------" << cstr
           << std::endl << "-----------" << std::endl;

#ifdef GL3_PROTOTYPES
    for( size_t i = 0; i < paths.size(); i++ )
    {
        glDeleteNamedStringARB(-1, paths[i].c_str());
    }
#endif

    glDeleteObjectARB( handle );
    return 0;
}


int GLSLShaders::cleanupOnError_( GLhandleARB shader1, GLhandleARB shader2,
                                  GLhandleARB shader3 )
{
    if( shader1 )
        glDeleteObjectARB( shader1 );

    if( shader2 )
        glDeleteObjectARB( shader2 );

    if( shader3 )
        glDeleteObjectARB( shader3 );

    if( program_ )
    {
        glDeleteObjectARB( program_ );
        program_ = 0;
    }

    const int glError = glGetError();
    if( glError == GL_NO_ERROR )
        return GL_INVALID_OPERATION; // most likely cause!?
    return glError;
}

std::string GLSLShaders::readShaderFile_( const std::string &shaderFile ) const
{
    std::ifstream shader( shaderFile.c_str() );
    std::stringstream shaderStr;
    shaderStr << shader.rdbuf();
    shader.close();
    return shaderStr.str();
}

int GLSLShaders::loadShaders( const ShaderData& shaderData )
{
    if( shadersLoaded_ )
        return true;

    program_ = glCreateProgramObjectARB();
    if( !program_ )
        return cleanupOnError_( 0 );

    GLhandleARB vertexShader=0;
    if( !shaderData.vShader.empty() )
    {
        vertexShader = loadShader_( shaderData.vShader, shaderData.paths,
                                    shaderData.glslCodes,
                                    GL_VERTEX_SHADER_ARB );
        if( !vertexShader )
            return cleanupOnError_( vertexShader );

        glAttachObjectARB( program_, vertexShader );
    }

    GLhandleARB fragmentShader=0;
    if(!shaderData.fShader.empty())
    {
        fragmentShader = loadShader_( shaderData.fShader, shaderData.paths,
                                      shaderData.glslCodes,
                                      GL_FRAGMENT_SHADER_ARB );
        if( !fragmentShader )
            return cleanupOnError_( vertexShader );

        glAttachObjectARB( program_, fragmentShader );
    }

    GLhandleARB geometryShader=0;
#ifdef GL3_PROTOTYPES
    if(!shaderData.gShader.empty())
    {
        geometryShader = loadShader_( shaderData.gShader, shaderData.paths,
                                      shaderData.glslCodes,
                                      GL_GEOMETRY_SHADER_ARB );
        if( !geometryShader )
            return cleanupOnError_( vertexShader, fragmentShader );

        glAttachObjectARB( program_, geometryShader );
    }
#endif

    glLinkProgramARB( program_ );

    GLint status;
    glGetObjectParameterivARB( program_, GL_OBJECT_LINK_STATUS_ARB,
                               &status );
    if( status == GL_FALSE )
    {
        printLog_( program_, "Linking" );
        return cleanupOnError_( vertexShader, fragmentShader, geometryShader );
    }

    shadersLoaded_ = true;
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
