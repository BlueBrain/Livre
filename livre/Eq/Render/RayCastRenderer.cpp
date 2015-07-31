/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/core/Render/GLSLShaders.h>
#include <livre/core/Render/RenderBrick.h>
#include <livre/core/Render/Frustum.h>
#include <livre/core/Render/TransferFunction1D.h>
#include <livre/core/Data/LODNode.h>
#include <livre/core/Maths/Maths.h>
#include <livre/core/Render/GLContext.h>
#include <livre/core/Render/GLWidget.h>
#include <livre/core/Render/View.h>

#include <livre/Eq/Render/Shaders/vertRayCast.glsl.h>
#include <livre/Eq/Render/Shaders/fragRayCast.glsl.h>
#include <livre/Eq/Render/RayCastRenderer.h>

#include <eq/eq.h>
#include <eq/gl.h>

namespace livre
{

#define glewGetContext() GLContext::glewGetContext()

namespace
{
std::string where( const char* file, const int line )
{
    return std::string( " in " ) + std::string( file ) + ":" +
           boost::lexical_cast< std::string >( line );
}
}

RayCastRenderer::RayCastRenderer( const uint32_t samples,
                                  const uint32_t componentCount,
                                  const GLenum gpuDataType,
                                  const GLint internalFormat )
    : Renderer( componentCount, gpuDataType, internalFormat )
    , _framebufferTexture(
        new eq::util::Texture( GL_TEXTURE_RECTANGLE_ARB, glewGetContext( )))
    , _nSamples( samples )
    , _transferFunctionTexture( 0 )
{
    TransferFunction1D< unsigned char > transferFunction;
    initTransferFunction( transferFunction );

    _shaders.reset( new GLSLShaders );

    // TODO: Add the shaders from resource directory
    const int error = _shaders->loadShaders( ShaderData( vertRayCast_glsl,
                                                         fragRayCast_glsl ));
    if( error != GL_NO_ERROR )
        LBTHROW( std::runtime_error( "Can't load glsl shaders: " +
                                     eq::glError( error ) +
                                     where( __FILE__, __LINE__ )));
}

RayCastRenderer::~RayCastRenderer()
{
    _framebufferTexture->flush();
}

void RayCastRenderer::initTransferFunction(
    const TransferFunction1Dc& transferFunction )
{
    assert( transferFunction.getNumChannels() == 4u );

    if( _transferFunctionTexture == 0 )
    {
        GLuint tfTexture = 0;
        glGenTextures( 1, &tfTexture );
        glBindTexture( GL_TEXTURE_1D, tfTexture );

        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        _transferFunctionTexture  = tfTexture;
    }
    glBindTexture( GL_TEXTURE_1D, _transferFunctionTexture );

    const UInt8Vector& transferFunctionData = transferFunction.getData();
    glTexImage1D(  GL_TEXTURE_1D, 0, GL_RGBA, GLsizei(transferFunctionData.size()/4u), 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, &transferFunctionData[ 0 ] );
}

bool RayCastRenderer::readFromFrameBuffer_( const GLWidget& glWidget,
                                            const View& view,
                                            const Frustum &frustum,
                                            const RenderBrick& renderBrick,
                                            Vector2i& screenPos )
{
    const Viewport& viewport = glWidget.getViewport( view );
    Vector2i minPos;
    Vector2i maxPos;

    renderBrick.getScreenCoordinates( frustum, viewport, minPos, maxPos );

    const Vector2i texSize = maxPos - minPos;

    const eq::PixelViewport pvp( minPos[ 0 ], minPos[ 1 ],
                                 texSize[ 0 ], texSize[ 1 ] );
    if( !pvp.hasArea( ))
        return false;

    _framebufferTexture->copyFromFrameBuffer( GL_RGBA, pvp );
    screenPos = minPos;
    return true;
}

void RayCastRenderer::onFrameStart_( const GLWidget& glWidget LB_UNUSED,
                                     const View& view LB_UNUSED,
                                     const Frustum &frustum,
                                     const RenderBricks& )
{
#ifdef LIVRE_DEBUG_RENDERING
    if( _usedTextures[0] != _usedTextures[1] )
    {
        std::cout << "Render ";
        std::copy( _usedTextures[1].begin(), _usedTextures[1].end(),
                   std::ostream_iterator< uint32_t >( std::cout, " " ));
        std::cout << std::endl;
    }
    _usedTextures[0].swap( _usedTextures[1] );
    _usedTextures[1].clear();
#endif

    glDisable( GL_LIGHTING );
    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );

    GLSLShaders::Handle program = _shaders->getProgram( );
    LBASSERT( program );

    // Enable shaders
    glUseProgram( program );
    GLint tParamNameGL;

    tParamNameGL = glGetUniformLocation( program, "invProjectionMatrix" );
    glUniformMatrix4fv( tParamNameGL, 1, false, frustum.getInvProjectionMatrix( ).array );

    tParamNameGL = glGetUniformLocation( program, "invModelViewMatrix" );
    glUniformMatrix4fv( tParamNameGL, 1, false, frustum.getInvModelViewMatrix( ).array );

    Vector4i viewport;
    glGetIntegerv( GL_VIEWPORT, viewport.array );
    tParamNameGL = glGetUniformLocation( program, "viewport" );
    glUniform4iv( tParamNameGL, 1, viewport.array );

    tParamNameGL = glGetUniformLocation( program, "depthRange" );

    Vector2f depthRange;
    glGetFloatv( GL_DEPTH_RANGE, depthRange.array );
    glUniform2fv( tParamNameGL, 1, depthRange.array );

    tParamNameGL = glGetUniformLocation( program, "worldEyePosition" );
    glUniform3fv( tParamNameGL, 1, frustum.getEyeCoords( ).array );

    // Disable shader
    glUseProgram( 0 );
}


void RayCastRenderer::renderBrick_( const GLWidget& glWidget,
                                    const View& view,
                                    const Frustum &frustum,
                                    const RenderBrick& renderBrick )
{
    GLSLShaders::Handle program = _shaders->getProgram( );
    LBASSERT( program );

    // Enable shaders
    glUseProgram( program );

    if( renderBrick.getTextureState( )->textureId == INVALID_TEXTURE_ID )
    {
        LBERROR << "Invalid texture for node : " << renderBrick.getLODNode( )->getNodeId( ) << std::endl;
        return;
    }

    GLint tParamNameGL = glGetUniformLocation( program, "aabbMin" );
    const ConstLODNodePtr& lodNodePtr = renderBrick.getLODNode( );
    glUniform3fv( tParamNameGL, 1, lodNodePtr->getWorldBox( ).getMin( ).array );

    tParamNameGL = glGetUniformLocation( program, "aabbMax" );
    glUniform3fv( tParamNameGL, 1, lodNodePtr->getWorldBox( ).getMax( ).array );

    tParamNameGL = glGetUniformLocation( program, "textureMin" );
    ConstTextureStatePtr texState = renderBrick.getTextureState( );
    glUniform3fv( tParamNameGL, 1, texState->textureCoordsMin.array );

    tParamNameGL = glGetUniformLocation( program, "textureMax" );
    glUniform3fv( tParamNameGL, 1, texState->textureCoordsMax.array );

    const Vector3f voxSize = renderBrick.getTextureState( )->textureSize / lodNodePtr->getWorldBox( ).getDimension( );
    tParamNameGL = glGetUniformLocation( program, "voxelSpacePerWorldSpace" );
    glUniform3fv( tParamNameGL, 1, voxSize.array );

    // Read from buffer and getthe position screen
    Vector2i screenPos;
    if( !readFromFrameBuffer_( glWidget, view, frustum, renderBrick, screenPos ))
    {
        glUseProgram( 0 );
        return;
    }

    // Put data to the shader
    tParamNameGL = glGetUniformLocation( program, "screenPos" );
    glUniform2iv( tParamNameGL, 1, screenPos.array );

    glActiveTexture( GL_TEXTURE2 );
    _framebufferTexture->bind( );
    _framebufferTexture->applyZoomFilter( eq::FILTER_LINEAR );
    _framebufferTexture->applyWrap( );

    tParamNameGL = glGetUniformLocation( program, "frameBufferTex" );
    glUniform1i( tParamNameGL, 2 );

    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_1D, _transferFunctionTexture ); //preintegrated values
    tParamNameGL = glGetUniformLocation( program, "transferFnTex" );
    glUniform1i( tParamNameGL, 1 ); //f-shader

    glActiveTexture( GL_TEXTURE0 );
    texState->bind( );
    tParamNameGL = glGetUniformLocation( program, "volumeTex" );
    glUniform1i( tParamNameGL, 0 ); //f-shader

    tParamNameGL = glGetUniformLocation(  program, "nSamples" );
    glUniform1i( tParamNameGL, _nSamples );

    const uint32_t refLevel = renderBrick.getLODNode( )->getRefLevel( );

    tParamNameGL = glGetUniformLocation(  program, "refLevel" );
    glUniform1i( tParamNameGL, refLevel );

#ifdef LIVRE_DEBUG_RENDERING
    _usedTextures[1].push_back( texState->textureId );
#endif
    renderBrick.drawBrick( true /* draw front */, false /* cull back */ );

    glUseProgram( 0 );
}

}
